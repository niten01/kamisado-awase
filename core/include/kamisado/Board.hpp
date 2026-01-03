#pragma once
#include "kamisado/BoardColoring.hpp"
#include "kamisado/BoardProps.hpp"
#include "kamisado/Config.hpp"
#include "kamisado/Player.hpp"
#include <bit>
#include <cstddef>
#include <optional>

namespace kamisado {

struct Cell {
  bool occupied : 1 { false };
  Player player : 1 { Player::White };
  Color color
      : std::bit_width(static_cast<size_t>(Color::Count) - 1ULL){};

  friend auto operator==(const Cell& lhs, const Cell& rhs)
      -> bool = default;
};

class Board {
  template <typename Self>
  [[nodiscard]] constexpr auto towerCoordRef(this Self&& self,
                                             Player player,
                                             Color towerColor)
      -> decltype(auto) {
    return std::forward<Self>(self).towers_[static_cast<size_t>(
        player)][static_cast<size_t>(towerColor)];
  }

public:
  explicit Board(BoardColoring coloring = BoardColoring::official());

  [[nodiscard]] auto coloring() const -> const BoardColoring&;
  [[nodiscard]] auto empty(Coord p) const -> bool;
  [[nodiscard]] constexpr auto size() const -> size_t {
    return board_.size();
  }
  [[nodiscard]] auto inBounds(Coord p) const -> bool;

  void place(Tower tower, Coord p);

  void move(Tower tower, Coord from, Coord to);

  [[nodiscard]] auto towerAt(Coord p) const -> std::optional<Tower>;

  [[nodiscard]] auto towerPos(Player player, Color towerColor) const
      -> Coord;

  void resetToInitial();

  static constexpr Coord::T WhiteHomeRow{ config::BoardSize - 1 };
  static constexpr Coord::T BlackHomeRow{ 0 };

private:
  BoardColoring coloring_;

  std::array<std::array<Cell, config::BoardSize>, config::BoardSize>
      board_;
  std::array<std::array<Coord, config::BoardSize>,
             static_cast<size_t>(Player::Count)>
      towers_;
};

} // namespace kamisado
