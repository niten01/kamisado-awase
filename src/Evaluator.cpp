#include "kamisado/Evaluator.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <cstdlib>

namespace kamisado {

namespace {

auto towerLost(Coord pos, Coord goal) -> bool {
  return std::abs(pos.row - goal.row) >= std::abs(pos.col - goal.col);
}

} // namespace

auto Evaluator::evaluate(const GameState& s, Player perspective) -> int {
  const auto status = s.terminalStatus();
  if (status.terminal) {
    assert(status.winner.has_value() && "Terminal with no winner");
    return (status.winner == perspective) ? s_MateScore : -s_MateScore;
  }

  const int my{ scoreSide(s, perspective) };
  const int op{ scoreSide(s, opposite(perspective)) };
  return my - op;
}

auto Evaluator::scoreSide(const GameState& s, Player perspective) -> int {
  const auto& board{ s.board() };

  int score{ 0 };

  constexpr int W_forward  = 30;  // reward being closer to goal row
  constexpr int W_align    = 12;  // reward being aligned with goal column
  constexpr int W_unreach  = 100; // heavy penalty per unreachable "step"
  constexpr int W_block    = 200; // heavy penalty per blocked goal row
  constexpr int W_mobility = 1;   // small mobility bonus
  constexpr int W_immobility  = 50; // immobility penalty
  constexpr int W_lowMobility = 25; // low mobility penalty
  constexpr int N_lowMobility = 5;  // low mobility threshold

  for (int c = 0; c < static_cast<int>(Color::Count); c++) {
    Color color{ static_cast<Color>(c) };
    const Coord pos{ board.towerPos(perspective, color) };
    assert(board.inBounds(pos) && "Tower not placed");

    const Coord goal{ s.goals().goal(perspective, color) };

    int rowsToGoal{ std::abs(pos.row - goal.row) };
    int colsToGoal{ std::abs(pos.col - goal.col) };
    int slack{ rowsToGoal - colsToGoal }; // <0 means lost tower
    int progress{ static_cast<int>(board.size() - rowsToGoal - 1) };

    score += (W_forward * progress);
    if (colsToGoal == 0 || colsToGoal == rowsToGoal) {
      score += W_align;
    }

    if (slack < 0) {
      score -= rowsToGoal == 0 ? W_block : W_unreach;
    }

    score +=
        W_mobility * MoveGen::towerMobility(board, perspective, color);
  }

  if (s.playerToMove() == perspective && s.forcedColor().has_value()) {
    const int m{ MoveGen::towerMobility(board, perspective,
                                        *s.forcedColor()) };
    if (m == 0) {
      score -= W_immobility;
    } else if (m <= N_lowMobility) {
      score -= W_lowMobility;
    }

    const Coord forcedTowerPos{ board.towerPos(perspective,
                                               *s.forcedColor()) };
    const Coord goal{ s.goals().goal(perspective, *s.forcedColor()) };

    if (towerLost(forcedTowerPos, goal)) {
      score -= W_immobility;
    }
  }

  return score;
}

} // namespace kamisado
