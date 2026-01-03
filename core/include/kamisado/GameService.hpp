#pragma once
#include "kamisado/GameState.hpp"
#include "kamisado/SearchEngine.hpp"
#include <unordered_set>

namespace kamisado {

class GameService {
public:
  GameService();
  ~GameService();
  [[nodiscard]] auto state() const -> const GameState&;
  [[nodiscard]] auto board() const -> const Board&;
  [[nodiscard]] auto playerToMove() const -> Player;
  [[nodiscard]] auto availableMoves() const -> const std::vector<Move>&;
  [[nodiscard]] auto turn() const -> int;
  [[nodiscard]] auto canMoveFrom(const Coord& coord) const -> bool;
  [[nodiscard]] auto lastMove() const -> std::optional<Move>;

  void makeMove(Move move);
  void setEngineCallback(
      std::function<void(const SearchEngine::Result&)> callback);
  void startEngineSearch();
  void stopEngine();
  void reset();

private:

private:
  int turn_{ 1 };
  GameState state_;
  Move lastMove_;
  std::vector<Move> availableMoves_;
  SearchEngine engine_;
  std::unordered_set<Coord, Coord::Hasher> canMoveFrom_;
};

} // namespace kamisado
