#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"

namespace kamisado {

namespace {

void addMovesFrom(const Board& board, Player player, Coord from,
                  std::vector<Move>& moves) {
  const int rowDelta = player == Player::White ? -1 : 1;
  std::array<int, 3> colDeltas{ -1, 0, 1 };
  for (auto colDelta : colDeltas) {
    Coord to{ from.row + rowDelta, from.col + colDelta };
    while (board.inBounds(to) && board.empty(to)) {
      moves.push_back(Move{ .from = from, .to = to, .isPass = false });
      to.row += rowDelta;
      to.col += colDelta;
    }
  }
}

auto getTowerPos(const Board& board, Player player, Color tower)
    -> Coord {
  Coord towerPos{ board.towerPos(player, tower) };
  assert(board.inBounds(towerPos) && "Tower not placed");
  return towerPos;
}

} // namespace

auto MoveGen::legalMoves(const GameState& s) -> std::vector<Move> {
  const auto status = s.terminalStatus();
  if (status.terminal) {
    return {};
  }

  const auto& board{ s.board() };
  const Player player{ s.playerToMove() };

  std::vector<Move> moves;
  moves.reserve(board.size() * board.size());

  if (s.forcedColor()) {
    Coord towerPos{ getTowerPos(board, player, *s.forcedColor()) };
    std::vector<Move> tmp;
    addMovesFrom(board, player, towerPos, tmp);

    if (tmp.empty()) {
      moves.push_back(Move::pass(towerPos));
    } else {
      moves = std::move(tmp);
    }
  } else {
    for (int c = 0; c < static_cast<int>(Color::Count); c++) {
      Coord towerPos{ getTowerPos(board, player, static_cast<Color>(c)) };
      addMovesFrom(board, player, towerPos, moves);
    }
  }

  return moves;
}

auto MoveGen::towerMobility(const Board& board, Player p, Color tower)
    -> int {
  auto towerPos{ board.towerPos(p, tower) };
  std::vector<Move> moves;
  moves.reserve(board.size() * board.size());
  addMovesFrom(board, p, towerPos, moves);
  return static_cast<int>(moves.size());
}
} // namespace kamisado
