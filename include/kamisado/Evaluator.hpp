#pragma once
#include "kamisado/GameState.hpp"

namespace kamisado {

class Evaluator {
public:
  static auto evaluate(const GameState& s, Player perspective) -> int;

  static auto isMateScore(int score) -> bool {
    return std::abs(score) > s_MateScore - s_MateBuffer;
  }

  static auto mateScore(bool win, int ply) -> int;
  static auto clampNonMateScore(int score) -> int;
  static auto normalize(int score) -> float;
  static auto formatScoreNorm(int score) -> std::string;
  static auto formatScore(int score) -> std::string;

private:
  static auto formatMate(int score) -> std::string;
  static auto scoreSide(const GameState& s, Player perspective) -> int;

private:
  static constexpr int s_MateScore{ 1'000'000 };
  static constexpr int s_MateBuffer{ 1'000 };

  // Normally something around max non-mate score
  static constexpr int s_ScoreScale{ 500 };
};

} // namespace kamisado
