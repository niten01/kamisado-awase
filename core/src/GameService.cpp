#include "kamisado/GameService.hpp"

namespace kamisado {

GameService::GameService()
    : state_{ Board{} } {
  reset();
}

GameService::~GameService() {
  engine_.stopSearch();
}

auto GameService::state() const -> const GameState& {
  return state_;
}

auto GameService::board() const -> const Board& {
  return state_.board();
}

auto GameService::playerToMove() const -> Player {
  return state_.playerToMove();
}

auto GameService::availableMoves() const -> const std::vector<Move>& {
  return availableMoves_;
}

auto GameService::turn() const -> int {
  return turn_;
}

auto GameService::canMoveFrom(const Coord& coord) const -> bool {
  return canMoveFrom_.contains(coord);
};

void GameService::reset() {
  state_ = GameState{ Board{ BoardColoring::official() } };
  turn_  = 1;
  engine_.stopSearch();
  engine_.setCallback([](auto&&) {
  });
  availableMoves_ = MoveGen::legalMoves(state_);
  canMoveFrom_.clear();
  for (auto&& move : availableMoves_) {
    canMoveFrom_.insert(move.from);
  }
}

void GameService::makeMove(Move move) {
  state_ = state_.apply(move);
  engine_.stopSearch();
  lastMove_ = move;
  turn_++;
  availableMoves_ = MoveGen::legalMoves(state_);
  canMoveFrom_.clear();
  for (auto&& move : availableMoves_) {
    canMoveFrom_.insert(move.from);
  }
}

auto GameService::lastMove() const -> std::optional<Move> {
  return turn_ == 1 ? std::nullopt : std::optional{ lastMove_ };
}

void GameService::setEngineCallback(
    std::function<void(const SearchEngine::Result&)> callback) {
  engine_.setCallback(std::move(callback));
}

void GameService::startEngineSearch() {
  engine_.startSearch(state_, config::MaxDepth);
}

void GameService::stopEngine() {
  engine_.stopSearch();
}

} // namespace kamisado
