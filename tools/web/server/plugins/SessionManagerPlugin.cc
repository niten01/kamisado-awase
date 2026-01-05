/**
 *
 *  SessionManagerPlugin.cc
 *
 */

#include "SessionManagerPlugin.h"
#include "kamisado/Player.hpp"
#include "sodium/utils.h"
#include "utils/Json.h"
#include "utils/Utils.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <json/value.h>
#include <random>
#include <sodium.h>

using namespace drogon;

namespace kamisado {

int SessionManagerPlugin::s_IDCounter = 0;

Session::Session(bool analysisEnabled)
    : s_{ std::make_unique<GameService>() },
      analysisEnabled_{ analysisEnabled },
      lastActive_{ std::chrono::system_clock::now() } {
}

SessionManagerPlugin::SessionManagerPlugin()
    : rng_{ std::random_device{}() } {
  sessionCleanerThread_ = std::jthread([this] {
    sessionCleaner();
  });
}

void SessionManagerPlugin::initAndStart(const Json::Value& config) {
}

void SessionManagerPlugin::shutdown() {
  running_ = false;
}

auto SessionManagerPlugin::create(SessionOptions options) -> int {
  auto id = s_IDCounter++;
  s_IDCounter %= s_MaxSessions;
  Session session{ options.analysisEnabled };
  sessions_.erase(id);
  auto [_, inserted] = sessions_.try_emplace(id, std::move(session));
  assert(inserted && "Session already exists");
  return id;
}

auto SessionManagerPlugin::get(int id) -> Session& {
  if (!sessions_.contains(id)) {
    throw SessionException("Session does not exist");
  }
  return sessions_.at(id);
}

auto SessionManagerPlugin::join(int id, Player player) -> Token {
  auto& session = get(id);
  std::array<std::byte, s_TokenLength> tokenRawBuf{};

  randombytes_buf(tokenRawBuf.data(), tokenRawBuf.size());

  // NOLINTNEXTLINE
  const size_t tokenLenWithNull{ sodium_base64_ENCODED_LEN(
      tokenRawBuf.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING) };
  Token token(tokenLenWithNull, '\0');

  sodium_bin2base64(
      token.data(), tokenLenWithNull,
      reinterpret_cast<unsigned char*>(tokenRawBuf.data()), // NOLINT
      tokenRawBuf.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING);
  token.resize(tokenLenWithNull - 1);

  TokenHash tokenHash{};
  crypto_generichash(
      reinterpret_cast<unsigned char*>(tokenHash.data()),
      tokenHash.size(),
      reinterpret_cast<unsigned char*>(token.data()), // NOLINT
      token.size(), nullptr, 0);

  if (tokens_.contains(tokenHash)) {
    throw SessionException("Token already exists");
  }
  tokens_[tokenHash] = std::make_pair(id, player);

  return token;
}

auto SessionManagerPlugin::auth(const Token& token) const
    -> std::optional<std::pair<int, Player>> {
  TokenHash tokenHash{};
  crypto_generichash(reinterpret_cast<unsigned char*>(tokenHash.data()),
                     tokenHash.size(),
                     reinterpret_cast<const unsigned char*>(token.data()),
                     token.size(), nullptr, 0);

  if (!tokens_.contains(tokenHash)) {
    return std::nullopt;
  }

  return tokens_.at(tokenHash);
}
void Session::subscribe(const drogon::WebSocketConnectionPtr& ws) {
  subscribers_.insert(ws);

  if (subscribers_.size() >= 2) {
    startGame();
  }
}
void Session::unsubscribe(const drogon::WebSocketConnectionPtr& ws) {
  subscribers_.erase(ws);
}

namespace {

auto toJson(const MoveEntry& entry) -> Json::Value {
  Json::Value out = toJson(entry.move);
  out["side"]     = toJson(entry.player);
  out["notation"] =
      fmt::format("{}-{}", out["from"].asString(), out["to"].asString());
  out["ts"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                  entry.ts.time_since_epoch())
                  .count();
  return out;
}

} // namespace

auto Session::stateJson() const -> Json::Value {
  Json::Value state;
  state["turnSide"] = toJson(s_->playerToMove());
  if (s_->lastMove().has_value()) {
    state["lastMove"] = toJson(*s_->lastMove());
  }
  state["moves"] = Json::Value(Json::arrayValue);
  for (const auto& moveEntry : moves_) {
    state["moves"].append(toJson(moveEntry));
  }
  state["legalMovesMap"] = Json::Value(Json::objectValue);
  auto& mm               = state["legalMovesMap"];
  for (const auto& move : s_->availableMoves()) {
    auto from = coordToFileRank(move.from);
    if (!mm.isMember(from)) {
      mm[from] = Json::Value(Json::arrayValue);
    }
    mm[from].append(toJson(move.to));
  }
  auto terminalStatus = s_->state().terminalStatus();
  state["terminal"]   = toJson(terminalStatus);
  state["board"]      = toJson(s_->board());
  return state;
}

void Session::pushMessage(const Json::Value& message) {
  for (const auto& ws : subscribers_) {
    ws->sendJson(message);
  }
}

auto Session::playerToMove() const -> Player {
  return s_->playerToMove();
}

auto Session::game() const -> const GameService& {
  return *s_;
}

void Session::makeMove(Move move) {
  const Board& board = s_->board();
  if (!board.inBounds(move.from) || !board.inBounds(move.to)) {
    throw SessionException("Move out of bounds");
  }

  auto&& legalMoves = s_->availableMoves();
  if (std::ranges::find(legalMoves, move) == legalMoves.end()) {
    throw SessionException("Illegal move");
  }

  s_->makeMove(move);
  moves_.emplace_back(move, playerToMove());
  Json::Value msg;
  msg["type"]    = "delta";
  msg["payload"] = stateJson();
  pushMessage(msg);
  lastActive_ = std::chrono::system_clock::now();

  if (s_->availableMoves().size() == 1 &&
      s_->availableMoves().front().isPass) {
    makeMove(s_->availableMoves().front());
    return;
  }

  if (analysisEnabled_ && !s_->state().terminalStatus().terminal) {
    s_->startEngineSearch();
  }
}

void Session::startGame() {
  Json::Value readyMessage;
  readyMessage["type"] = "ready";
  pushMessage(readyMessage);

  s_->setEngineCallback([this](const SearchEngine::Result& result) {
    if (!result.bestMove.has_value()) {
      return;
    }

    Json::Value msg;
    msg["type"]                = "analysis";
    msg["payload"]             = Json::objectValue;
    msg["payload"]["bestMove"] = toJson(*result.bestMove);
    auto whiteScore =
        result.score * (s_->playerToMove() == Player::White ? 1 : -1);
    msg["payload"]["advantageWhite"] = Evaluator::normalize(whiteScore);
    msg["payload"]["formattedScoreWhite"] =
        Evaluator::formatScoreNorm(whiteScore);
    msg["payload"]["formattedScoreBlack"] =
        Evaluator::formatScoreNorm(-whiteScore);
    pushMessage(msg);
  });

  if (analysisEnabled_) {
    s_->startEngineSearch();
  }
}

auto Session::analysisEnabled() const -> bool {
  return analysisEnabled_;
}

auto SessionManagerPlugin::randomFreeSlot(int id) -> Player {
  auto predicate = [id](const auto& kv) {
    return kv.second.first == id;
  };

  if (std::ranges::count_if(tokens_, predicate) >= 2) {
    throw SessionException("No free slots");
  }

  auto it = std::ranges::find_if(tokens_, predicate);

  if (it == tokens_.end()) {
    return std::uniform_int_distribution<>{ 0, 1 }(rng_) ? Player::White
                                                         : Player::Black;
  }

  return opposite(it->second.second);
}

void SessionManagerPlugin::leave(int id, Player player) {
  auto it = std::ranges::find_if(tokens_, [id, player](const auto& kv) {
    return kv.second == std::make_pair(id, player);
  });
  if (it != tokens_.end()) {
    tokens_.erase(it);
  }
}

void SessionManagerPlugin::sessionCleaner() {
  while (running_) {
    std::this_thread::sleep_for(std::chrono::seconds(60));

    auto now = std::chrono::system_clock::now();
    for (auto it = sessions_.begin(); it != sessions_.end();) {
      auto&& [id, session] = *it;
      auto delta = std::chrono::duration_cast<std::chrono::minutes>(
                       now - session.lastActive())
                       .count();
      if (delta > 30) {
        it = sessions_.erase(it);
        std::erase_if(tokens_, [id](const auto& kv) {
          return kv.second.first == id;
        });
      } else {
        it++;
      }
    }
  }
}

} // namespace kamisado
