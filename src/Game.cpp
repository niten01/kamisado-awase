#include "kamisado/Game.hpp"
#include "kamisado/BoardColoring.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <algorithm>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

namespace kamisado {

namespace {
auto color(Color c) -> raylib::Color {
  switch (c) {
  case Color::Brown:
    return raylib::Color{ 0x572600FF };
  case Color::Green:
    return raylib::Color{ 0x009057FF };
  case Color::Red:
    return raylib::Color{ 0xD23339FF };
  case Color::Yellow:
    return raylib::Color{ 0xE3C301FF };
  case Color::Pink:
    return raylib::Color{ 0xD2719EFF };
  case Color::Purple:
    return raylib::Color{ 0x6F3787FF };
  case Color::Blue:
    return raylib::Color{ 0x006BACFF };
  case Color::Orange:
    return raylib::Color{ 0xD77522FF };
  default:
    assert(false && "Unknown color");
  }
}

auto rectCenter(const raylib::Rectangle& rect) -> raylib::Vector2 {
  return { rect.x + (rect.width / 2), rect.y + (rect.height / 2) };
}
} // namespace

Game::Game()
    : window_{ s_WindowSize, s_WindowSize, "Mu-Torere" },
      gameState_{ Board{ BoardColoring::official() } } {
  SetTargetFPS(60);

  boardRect_ = { .x      = 0,
                 .y      = s_WindowSize - s_BoardSize,
                 .width  = s_BoardSize,
                 .height = s_BoardSize };

  auto tileSize =
      boardRect_.width / static_cast<float>(config::BoardSize);
  for (int row = 0; row < static_cast<int>(tiles_.size()); row++) {
    for (int col = 0; col < static_cast<int>(tiles_.size()); col++) {
      tiles_[row][col] = {
        .x      = boardRect_.x + (static_cast<float>(col) * tileSize),
        .y      = boardRect_.y + (static_cast<float>(row) * tileSize),
        .width  = tileSize,
        .height = tileSize
      };
    }
  }
}

void Game::run() {
  gameState_      = GameState{ Board{ BoardColoring::official() } };
  availableMoves_ = MoveGen::legalMoves(gameState_);
  turn_           = 1;
  while (!window_.ShouldClose()) {
    updateLogic();
    draw();
  }
}

void Game::updateHumanMove() {
  if (availableMoves_.size() == 1 && availableMoves_[0].isPass) {
    makeMove(availableMoves_[0]);
  }

  if (!raylib::Mouse::IsButtonPressed(::MOUSE_BUTTON_LEFT)) {
    return;
  }

  auto posOpt{ testHitCoord(raylib::Mouse::GetPosition()) };
  if (!posOpt) {
    resetSelection();
    return;
  }
  Coord pos{ *posOpt };

  const Board& board{ gameState_.board() };

  if (auto tower{ board.towerAt(pos) };
      tower && tower->owner == humanPlayer_) {
    selectedTile_ = pos;
    drawMoves_.clear();
    std::ranges::copy_if(availableMoves_, std::back_inserter(drawMoves_),
                         [&](const auto& m) {
                           return m.from == pos;
                         });
    if (drawMoves_.empty()) {
      resetSelection();
    }
  } else if (board.empty(pos) && selectedTile_) {
    auto moveIt{ std::ranges::find_if(drawMoves_, [&](const auto& m) {
      return m.to == pos;
    }) };
    if (moveIt == drawMoves_.end()) {
      resetSelection();
      return;
    }
    makeMove(*moveIt);
    resetSelection();
  } else {
    resetSelection();
    return;
  }
}

void Game::updateEngineMove() {
  if (!engine_.running()) {
    engine_.startSearch(gameState_, config::MaxDepth);
  }

  if (raylib::Keyboard::IsKeyPressed(::KEY_SPACE)) {
    engine_.stopSearch();
    auto result{ engine_.currentBest() };
    std::cout << "score: " << result->score << '\n';
    if (result->bestMove) {
      makeMove(*result->bestMove);
    }
  }
}

void Game::updateLogic() {
  auto status{ gameState_.terminalStatus() };
  if (status.terminal) {
    std::cout << "Winner: " << (int)status.winner.value_or(Player::Count)
              << '\n';
  }

  if (gameState_.playerToMove() == humanPlayer_) {
    updateHumanMove();
  } else {
    updateEngineMove();
  }
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(::RAYWHITE);

  drawBoard();

  EndDrawing();
}

void Game::drawBoard() {
  const Board& board = gameState_.board();
  auto hoveredCoordOpt{ testHitCoord(raylib::Mouse::GetPosition()) };

  // board
  for (int row = 0; row < static_cast<int>(board.size()); row++) {
    for (int col = 0; col < static_cast<int>(board.size()); col++) {
      Coord pos{ row, col };
      auto tile = tiles_[row][col];
      raylib::Color c{ color(board.coloring().at(pos)) };
      bool hovered{ hoveredCoordOpt && *hoveredCoordOpt == pos };
      if (selectedTile_ && *selectedTile_ == pos) {
        c = ::ColorBrightness(c, -.5F);
      } else if (hovered) {
        c = ::ColorBrightness(c, .1F);
      }
      ::DrawRectangleRec(tile, c);
      if (hovered) {
        ::DrawRectangleLinesEx(tile, 2, ::DARKGRAY);
      }
    }
  }

  // last move
  if (turn_ > 1) {
    drawMove(lastMove_, raylib::Color{ 0xFFFFFF55 });
  }

  // best move
  if (engine_.running()) {
    auto resultOpt{ engine_.currentBest() };
    if (resultOpt && resultOpt->bestMove) {
      Move bestMove{ *resultOpt->bestMove };
      drawMove(bestMove, ::BLUE);
    }
  }

  // towers
  for (int row = 0; row < static_cast<int>(board.size()); row++) {
    for (int col = 0; col < static_cast<int>(board.size()); col++) {
      Coord pos{ row, col };
      auto tile       = tiles_[row][col];
      auto tileCenter = rectCenter(tile);
      if (auto tower{ board.towerAt(pos) }) {
        raylib::Color baseColor{ tower->owner == Player::Black
                                     ? ::BLACK
                                     : ::WHITE };
        float r{ (tile.width / 2) - 5 };
        float rInner{ 2 * r / 3 };
        if (canMoveFrom_.contains(pos)) {
          ::DrawRing(tileCenter, (tile.width / 2) - 3, tile.width / 2, 0,
                     360, 50, ::WHITE);
        }
        ::DrawCircleV(tileCenter, r, baseColor);
        ::DrawCircleLinesV(tileCenter, r, ::GRAY);
        ::DrawCircleV(tileCenter, rInner, color(tower->color));
      }
    }
  }

  // available moves
  for (auto&& move : drawMoves_) {
    auto tile{ tiles_[move.to.row][move.to.col] };
    auto center{ rectCenter(tile) };
    float r{ tile.width / 2 };
    DrawCircleV(center, r, s_MoveColor);
  }
}

auto Game::testHitCoord(raylib::Vector2 pos) const
    -> std::optional<Coord> {
  for (int row = 0; row < static_cast<int>(tiles_.size()); row++) {
    for (int col = 0; col < static_cast<int>(tiles_.size()); col++) {
      if (!::CheckCollisionPointRec(pos, tiles_[row][col])) {
        continue;
      }
      return Coord{ row, col };
    }
  }

  return std::nullopt;
}

void Game::resetSelection() {
  selectedTile_.reset();
  drawMoves_.clear();
}

void Game::makeMove(Move m) {
  gameState_ = gameState_.apply(m);
  lastMove_  = m;
  turn_++;
  availableMoves_ = MoveGen::legalMoves(gameState_);
  canMoveFrom_.clear();
  for (auto&& move : availableMoves_) {
    canMoveFrom_.insert(move.from);
  }
  resetSelection();
}

void Game::drawMove(Move m, raylib::Color c) {
  auto fromCenter{ rectCenter(tiles_[m.from.row][m.from.col]) };
  auto toCenter{ rectCenter(tiles_[m.to.row][m.to.col]) };
  DrawLineEx(fromCenter, toCenter, 2, c);
}
} // namespace kamisado
