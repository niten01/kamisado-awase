#pragma once
#include "kamisado/GameState.hpp"

namespace kamisado {

class Evaluator {
public:
  static auto evaluate(const GameState& s, Player perspective) -> int;

  static constexpr auto isMateScore(int score) -> bool {
    return std::abs(score) >= s_MateScore;
  }

private:
  static auto scoreSide(const GameState& s, Player perspective) -> int;

private:
  static constexpr int s_MateScore{ 1'000'000 };
};

} // namespace kamisado
