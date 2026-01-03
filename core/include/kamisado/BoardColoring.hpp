#pragma once
#include "kamisado/BoardProps.hpp"
#include "kamisado/Config.hpp"
#include <array>

namespace kamisado {

class BoardColoring {
public:
  explicit constexpr BoardColoring(auto colors)
      : colors_{ std::move(colors) } {
  }

  [[nodiscard]] constexpr auto at(Coord p) const -> Color {
    assert(p.row < colors_.size() && p.col < colors_.size() &&
           "out of bounds");
    return colors_[p.row][p.col];
  }

  template <typename T, typename U>
  [[nodiscard]] constexpr auto at(T r, U c) const -> Color {
    return at(Coord{ r, c });
  }

  static constexpr auto official() -> BoardColoring {
    return BoardColoring{ config::OfficialBoardColors };
  }

private:
  std::array<std::array<Color, config::BoardSize>, config::BoardSize>
      colors_{};
};

} // namespace kamisado
