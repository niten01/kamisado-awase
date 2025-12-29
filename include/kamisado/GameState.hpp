#pragma once
#include "kamisado/Board.hpp"
#include "kamisado/BoardProps.hpp"
#include "kamisado/Goals.hpp"
#include "kamisado/Move.hpp"
#include "kamisado/Outcome.hpp"
#include "kamisado/Player.hpp"
#include <cstdint>
#include <vector>

namespace kamisado {

class GameState {
public:
  explicit GameState(Board board);

  [[nodiscard]] auto board() const -> const Board&;
  [[nodiscard]] auto playerToMove() const -> Player;
  [[nodiscard]] auto forcedColor() const -> std::optional<Color>;
  [[nodiscard]] auto hash() const -> uint64_t;
  [[nodiscard]] auto goals() const -> Goals;

  [[nodiscard]] auto terminalStatus() const -> Outcome;

  [[nodiscard]] auto apply(Move move) const -> GameState;

private:
  void setForcedColor(Color newForcedColor);
  void toggleSideToMove();

  void applyInPlace(Move move);
  static auto recalculateHash(const GameState& s) -> uint64_t;

private:
  Board board_;
  Player playerToMove_{ Player::White };
  std::optional<Color> forcedColor_;
  uint64_t hash_{};
  std::vector<uint64_t> history_;
  Goals goals_;
};

} // namespace kamisado
