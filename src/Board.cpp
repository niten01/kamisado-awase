#include "kamisado/Board.hpp"

namespace kamisado {

Board::Board(BoardColoring coloring)
    : coloring_{ coloring } {
  resetToInitial();
}

auto Board::coloring() const -> const BoardColoring& {
  return coloring_;
}

auto Board::empty(Coord p) const -> bool {
  return !board_[p.row][p.col].occupied;
}

void Board::place(Tower tower, Coord p) {
  assert(empty(p) && "Cell already occupied");
  board_[p.row][p.col]                    = { .occupied = true,
                                              .player   = tower.owner,
                                              .color    = tower.color };
  towerCoordRef(tower.owner, tower.color) = p;
}

void Board::move(Tower tower, Coord from, Coord to) {
  assert(from.col < board_.size() && from.row < board_.size() &&
         to.col < board_.size() && to.row < board_.size() &&
         "Out of bounds");
  assert(empty(to) && "Cell already occupied");

  Cell cellWithTower = { .occupied = true,
                         .player   = tower.owner,
                         .color    = tower.color };
  assert(board_[from.row][from.col] == cellWithTower &&
         "No such tower on 'from'");

  board_[from.row][from.col]              = { .occupied = false };
  board_[to.row][to.col]                  = cellWithTower;
  towerCoordRef(tower.owner, tower.color) = to;
}

auto Board::towerAt(Coord p) const -> std::optional<Tower> {
  Cell cell = board_[p.row][p.col];
  if (!cell.occupied) {
    return std::nullopt;
  }
  return Tower{ .owner = cell.player, .color = cell.color };
}

auto Board::towerPos(Player player, Color towerColor) const -> Coord {
  return towerCoordRef(player, towerColor);
}

void Board::resetToInitial() {
  board_  = {};
  towers_ = {};

  for (int i = 0; i < static_cast<int>(board_.size()); i++) {
    auto blackRow       = Coord{ BlackHomeRow, static_cast<Coord::T>(i) };
    auto whiteRow       = Coord{ WhiteHomeRow, static_cast<Coord::T>(i) };
    Color blackRowColor = coloring_.at(blackRow);
    Color whiteRowColor = coloring_.at(whiteRow);

    place(Tower{ .owner = Player::Black, .color = blackRowColor },
          blackRow);
    place(Tower{ .owner = Player::White, .color = whiteRowColor },
          whiteRow);
  }
}

auto Board::size() const -> size_t {
  return board_.size();
}
auto Board::inBounds(Coord p) const -> bool {
  return p.row < size() && p.col < size();
}
} // namespace kamisado
