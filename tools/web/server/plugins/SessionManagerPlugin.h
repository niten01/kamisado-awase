/**
 *
 *  SessionManagerPlugin.h
 *
 */

#pragma once

#include "drogon/WebSocketConnection.h"
#include "kamisado/GameService.hpp"
#include "kamisado/Move.hpp"
#include "sodium/crypto_generichash.h"
#include <drogon/plugins/Plugin.h>
#include <list>
#include <memory>
#include <random>
#include <stdexcept>

namespace kamisado {

using Token     = std::string;
using TokenHash = std::array<unsigned char, crypto_generichash_BYTES>;

struct MoveEntry {
  MoveEntry(Move move, Player player)
      : move{ move },
        player{ player } {
    ts = std::chrono::system_clock::now();
  }

  Move move;
  Player player;
  std::chrono::system_clock::time_point ts;
};

class Session {
public:
  explicit Session(bool analysisEnabled);

  auto game() const -> const GameService&;
  auto analysisEnabled() const -> bool;
  [[nodiscard]] auto stateJson() const -> Json::Value;
  auto playerToMove() const -> Player;

  void subscribe(const drogon::WebSocketConnectionPtr& ws);
  void unsubscribe(const drogon::WebSocketConnectionPtr& ws);
  void makeMove(Move move);


private:
  void pushMessage(const Json::Value& message);
  void startGame();

private:
  std::unique_ptr<GameService> s_;
  bool analysisEnabled_{ false };
  std::list<MoveEntry> moves_;
  std::unordered_set<drogon::WebSocketConnectionPtr> subscribers_;
};

struct SessionOptions {
  bool analysisEnabled{ false };
};

struct SessionException : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class SessionManagerPlugin : public drogon::Plugin<SessionManagerPlugin> {
public:
  SessionManagerPlugin();
  void initAndStart(const Json::Value& config) override;

  void shutdown() override;

  [[nodiscard]] auto create(SessionOptions options) -> int;
  auto get(int id) -> Session&;

  [[nodiscard]] auto auth(const Token& token) const
      -> std::optional<std::pair<int, Player>>;
  [[nodiscard]] auto join(int id, Player player) -> Token;
  void leave(int id, Player player);
  auto randomFreeSlot(int id) -> Player;

private:
  constexpr static int s_MaxSessions    = 10'000;
  static constexpr size_t s_TokenLength = 32;
  static int s_IDCounter;

  struct TokenHashHasher {
    auto operator()(const TokenHash& h) const -> std::size_t {
      std::string_view hStr{ reinterpret_cast<const char*>(h.data()),
                             h.size() };
      return std::hash<std::string_view>()(hStr);
    }
  };

  std::mt19937 rng_;
  std::unordered_map<int, Session> sessions_;
  std::unordered_map<TokenHash, std::pair<int, Player>, TokenHashHasher>
      tokens_;
};
} // namespace kamisado
