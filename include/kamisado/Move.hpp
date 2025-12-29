#pragma once
#include "kamisado/BoardProps.hpp"
#include <fmt/format.h>
#include <ostream>

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

  friend auto operator<<(std::ostream& os, const Move& move)
      -> std::ostream& {
    if (move.isPass) {
      os << "pass";
    } else {
      os << fmt::format("[{},{}]->[{},{}]", move.from.row, move.from.col,
                        move.to.row, move.to.col);
    }
    return os;
  }
};

inline auto format_as(Move move) -> std::string {
  if (move.from == Coord{} || move.to == Coord{}) {
    return "X";
  }

  if (move.isPass) {
    return "pass";
  }

  return fmt::format("[{},{}]->[{},{}]", move.from.row, move.from.col,
                     move.to.row, move.to.col);
}

} // namespace kamisado
