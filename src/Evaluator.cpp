#include "kamisado/Evaluator.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <cstdlib>

namespace kamisado {

namespace {

auto advance(const Board& board, Player perspective, Coord towerPos)
    -> int {
  return perspective == Player::White
             ? std::abs(towerPos.row - Board::BlackHomeRow)
             : std::abs(towerPos.row - Board::WhiteHomeRow);
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
  int bestAdvance{ 0 };
  int sumAdvance{ 0 };
  int mobility{ 0 };

  for (int c = 0; c < static_cast<int>(Color::Count); c++) {
    Color color{ static_cast<Color>(c) };
    const Coord pos{ board.towerPos(perspective, color) };
    assert(board.inBounds(pos) && "Tower not placed");
    const int adv{ advance(board, perspective, pos) };

    bestAdvance = std::max(bestAdvance, adv);
    sumAdvance += adv;
    mobility += MoveGen::towerMobility(board, perspective, color);
  }

  int forcedPenalty{ 0 };
  if (s.playerToMove() == perspective && s.forcedColor().has_value()) {
    const int m{ MoveGen::towerMobility(board, perspective,
                                        *s.forcedColor()) };
    forcedPenalty = (m == 0) ? 25
                             : (s_LowMobilityNumMoves -
                                std::min(m, s_LowMobilityNumMoves));
  }

  return (bestAdvance * s_BestAdvanceMult) +
         (sumAdvance * s_SumAdvanceMult) + (mobility * s_MobilityMult) -
         (forcedPenalty * s_ForcedPenaltyMult);
}

} // namespace kamisado
