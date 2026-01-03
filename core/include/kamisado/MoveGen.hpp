#pragma once
#include "kamisado/GameState.hpp"
#include "kamisado/Move.hpp"
#include <vector>

namespace kamisado {

struct MoveGen {
  static auto legalMoves(const GameState& s) -> std::vector<Move>;

  static auto towerMobility(const Board& board, Player p, Color tower)
      -> int;
};

} // namespace kamisado
