#pragma once
#include "kamisado/GameState.hpp"

namespace kamisado {

class Evaluator {
public:
  static auto evaluate(const GameState& s, Player perspective) -> int;

private:
  static auto scoreSide(const GameState& s, Player perspective) -> int;

private:
  static constexpr int s_MateScore{ 1'000'000 };
  static constexpr int s_LowMobilityNumMoves{ 3 };
  static constexpr int s_BestAdvanceMult{ 40 };
  static constexpr int s_SumAdvanceMult{ 8 };
  static constexpr int s_MobilityMult{ 2 };
  static constexpr int s_ForcedPenaltyMult{ 15 };
};

} // namespace kamisado
