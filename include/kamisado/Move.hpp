#pragma once
#include "kamisado/BoardProps.hpp"

namespace kamisado {

struct Move {
  Coord from;
  Coord to;
  bool isPass{ false };

  static auto pass(Coord where) -> Move {
    return Move{ .from = where, .to = where, .isPass = true };
  }

  friend auto operator==(const Move& lhs, const Move& rhs)
      -> bool = default;
};

} // namespace kamisado
