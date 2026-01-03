#pragma once
#include "kamisado/Player.hpp"
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>

namespace kamisado {

enum class Color : uint8_t {
  Brown,
  Green,
  Red,
  Yellow,
  Pink,
  Purple,
  Blue,
  Orange,
  Count
};

struct Coord {
  using T = uint8_t;

  Coord() = default;
  template <typename T, typename U>
  explicit constexpr Coord(T row, U col)
      : row(row),
        col(col) {
  }

  T row{ std::numeric_limits<T>::max() };
  T col{ std::numeric_limits<T>::max() };

  friend auto operator==(const Coord& l, const Coord& r)
      -> bool = default;

  struct Hasher {
    auto operator()(const Coord& c) const -> std::size_t {
      return std::hash<int>()(c.row) ^ std::hash<int>()(c.col);
    }
  };
};

struct Tower {
  Player owner;
  Color color;
};

} // namespace kamisado
