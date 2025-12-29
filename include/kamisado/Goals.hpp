#pragma once
#include "kamisado/Board.hpp"
#include "kamisado/BoardColoring.hpp"
#include "kamisado/BoardProps.hpp"

namespace kamisado {

class Goals {
public:
  explicit constexpr Goals(BoardColoring coloring) {
    for (size_t p = 0; p < static_cast<size_t>(Player::Count); ++p) {
      Player player{ static_cast<Player>(p) };
      int goalRow{ row(player) };
      int homeRow{ row(opposite(player)) };
      for (size_t homeCol = 0; homeCol < config::BoardSize; ++homeCol) {
        Color towerColor{ coloring.at(homeRow, homeCol) };
        bool found{ false };
        for (size_t goalCol = 0; goalCol < config::BoardSize; ++goalCol) {
          if (coloring.at(goalRow, goalCol) == towerColor) {
            goalCols_[p][static_cast<size_t>(towerColor)] = goalCol;
            found                                         = true;
            break;
          }
        }
        assert(found && "Goal not found");
      }
    }
  }

  [[nodiscard]] constexpr auto col(Player p, Color c) const -> Coord::T {
    return goalCols_[static_cast<size_t>(p)][static_cast<size_t>(c)];
  }

  // NOLINTNEXTLINE
  [[nodiscard]] constexpr auto row(Player p) const -> Coord::T {
    return p == Player::White ? Board::BlackHomeRow : Board::WhiteHomeRow;
  }

  [[nodiscard]] constexpr auto goal(Player p, Color c) const -> Coord {
    return Coord{ row(p), col(p, c) };
  }

private:
  std::array<std::array<Coord::T, config::BoardSize>,
             static_cast<size_t>(Player::Count)>
      goalCols_{};
};

} // namespace kamisado
