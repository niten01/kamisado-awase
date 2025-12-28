#pragma once
#include "kamisado/BoardProps.hpp"
#include <array>
#include <cstddef>

namespace kamisado::config {

constexpr size_t BoardSize = 8;

// clang-format off
constexpr auto OfficialBoardColoring = std::array{
  std::array{ Color::Brown, Color::Purple,Color::Blue,  Color::Yellow,Color::Pink,  Color::Green, Color::Red,   Color::Orange },
  std::array{ Color::Green, Color::Brown, Color::Yellow,Color::Red,   Color::Purple,Color::Pink,  Color::Orange,Color::Blue },
  std::array{ Color::Red,   Color::Yellow,Color::Brown, Color::Green, Color::Blue,  Color::Orange,Color::Pink,  Color::Purple },
  std::array{ Color::Yellow,Color::Blue,  Color::Purple,Color::Brown, Color::Orange,Color::Red,   Color::Green, Color::Pink },
  std::array{ Color::Pink,  Color::Green, Color::Red,   Color::Orange,Color::Brown, Color::Purple,Color::Blue,  Color::Yellow },
  std::array{ Color::Purple,Color::Pink,  Color::Orange,Color::Blue,  Color::Green, Color::Brown, Color::Yellow,Color::Red },
  std::array{ Color::Blue,  Color::Orange,Color::Pink,  Color::Purple,Color::Red,   Color::Yellow,Color::Brown, Color::Green },
  std::array{ Color::Orange,Color::Red,   Color::Green, Color::Pink,  Color::Yellow,Color::Blue,  Color::Purple,Color::Brown },
};
// clang-format on

} // namespace kamisado::config
