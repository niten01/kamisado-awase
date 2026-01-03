#include "engine-gui/Game.hpp"
#include "kamisado/BoardColoring.hpp"
#include "kamisado/Config.hpp"
#include "kamisado/Evaluator.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <algorithm>
#include <cassert>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <rlImGui.h>

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
    return -1;
  }
}

auto rectCenter(const raylib::Rectangle& rect) -> raylib::Vector2 {
  return { rect.x + (rect.width / 2), rect.y + (rect.height / 2) };
}

} // namespace

Game::Game()
    : window_{ s_WindowSize, s_WindowSize, "Mu-Torere" } {
  SetTargetFPS(60);
  rlImGuiSetup(true);

  s_.setEngineCallback([this](const SearchEngine::Result& result) {
    bestMove_  = result.bestMove;
    bestScore_ = result.score;
  });

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

Game::~Game() {
  rlImGuiShutdown();
}

void Game::run() {
  restartAs(humanPlayer_); // default human
  while (!window_.ShouldClose()) {
    updateLogic();
    draw();
  }
}

void Game::updateHumanMove() {
  if (!raylib::Mouse::IsButtonPressed(::MOUSE_BUTTON_LEFT)) {
    return;
  }

  auto posOpt{ testHitCoord(raylib::Mouse::GetPosition()) };
  if (!posOpt) {
    resetSelection();
    return;
  }
  Coord pos{ *posOpt };

  const Board& board{ s_.board() };

  if (auto tower{ board.towerAt(pos) };
      tower && tower->owner == humanPlayer_) {
    selectTile(pos);
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
  engineTimer_ += GetFrameTime();

  if (raylib::Keyboard::IsKeyPressed(::KEY_SPACE) ||
      engineTimer_ >= engineMaxTimeSeconds_) {
    engineTimer_ = 0;
    if (bestMove_) {
      makeMove(*bestMove_);
    }
  }
}

void Game::updateLogic() {
  if (state_ == State::GameOver) {
    return;
  }

  auto status{ s_.state().terminalStatus() };
  if (status.terminal) {
    state_ = State::GameOver;
    s_.stopEngine();
  }

  if (s_.availableMoves().size() == 1 &&
      (s_.availableMoves()[0].isPass ||
       s_.playerToMove() != humanPlayer_)) {
    makeMove(s_.availableMoves()[0]);
    return;
  }

  if (s_.playerToMove() == humanPlayer_) {
    updateHumanMove();
  } else {
    updateEngineMove();
  }
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(::RAYWHITE);

  drawBoard();
  drawAdvantageBar();

  if (state_ == State::GameOver) {
    assert(s_.state().terminalStatus().winner.has_value() &&
           "Game over with no winner");
    const int fontSize{ 100 };
    std::string winStr{ fmt::format(
        "{} wins!", s_.state().terminalStatus().winner == Player::White
                        ? "White"
                        : "Black") };
    DrawText(winStr.c_str(),
             s_WindowSize / 2 - MeasureText(winStr.c_str(), fontSize) / 2,
             s_WindowSize / 2 - fontSize, fontSize, ::DARKGRAY);
  }

  drawGUI();

  EndDrawing();
}

void Game::drawBoard() {
  const Board& board = s_.board();
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
  if (auto lastMove{ s_.lastMove() }) {
    drawMove(*lastMove, raylib::Color{ 0xFFFFFF55 });
  }

  // best move
  if (showMoves_ != ShowMoves::None) {
    if (bestMove_) {
      if ((showMoves_ == ShowMoves::Engine &&
           s_.playerToMove() == opposite(humanPlayer_)) ||
          showMoves_ == ShowMoves::All) {
        Move bestMove{ *bestMove_ };
        drawMove(bestMove, ::BLUE);
      }
    }
  }

  // towers
  drawTowers();

  // available moves
  for (auto&& move : drawMoves_) {
    auto tile{ tiles_[move.to.row][move.to.col] };
    auto center{ rectCenter(tile) };
    float r{ tile.width / 2 };
    DrawCircleV(center, r, s_MoveColor);
  }
}

void Game::drawTowers() {
  const Board& board = s_.board();
  for (int row = 0; row < static_cast<int>(board.size()); row++) {
    for (int col = 0; col < static_cast<int>(board.size()); col++) {
      Coord pos{ row, col };
      auto tile       = tiles_[row][col];
      auto tileCenter = rectCenter(tile);
      if (auto tower{ board.towerAt(pos) }) {
        raylib::Color baseColor{ tower->owner == Player::Black
                                     ? s_BlackColor
                                     : s_WhiteColor };
        float r{ (tile.width / 2) - 5 };
        float rInner{ 2 * r / 3 };
        if (s_.canMoveFrom(pos)) {
          ::DrawRing(tileCenter, (tile.width / 2) - 3, tile.width / 2, 0,
                     360, 50, ::WHITE);
        }
        ::DrawCircleV(tileCenter, r, baseColor);
        ::DrawCircleLinesV(tileCenter, r, ::GRAY);
        ::DrawCircleV(tileCenter, rInner, color(tower->color));
      }
    }
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
  s_.makeMove(m);
  s_.startEngineSearch();

  resetSelection();
  assert(s_.state().forcedColor().has_value() &&
         "Move 2+ should have forced color");
  selectTile(s_.board().towerPos(s_.state().playerToMove(),
                                 *s_.state().forcedColor()));
}

void Game::drawMove(Move m, raylib::Color c) {
  auto fromCenter{ rectCenter(tiles_[m.from.row][m.from.col]) };
  auto toCenter{ rectCenter(tiles_[m.to.row][m.to.col]) };
  DrawLineEx(fromCenter, toCenter, 2, c);
}

void Game::flipBoard() {
  for (auto& tile : tiles_) {
    std::ranges::reverse(tile);
  }

  std::ranges::reverse(tiles_);
  flipped_ = !flipped_;
}

void Game::drawGUI() {
  rlImGuiBegin();

  ImGui::Begin("Controls");

  if (ImGui::Button("Restart as white")) {
    restartAs(Player::White);
  }
  ImGui::SameLine();
  if (ImGui::Button("Restart as black")) {
    restartAs(Player::Black);
  }

  if (ImGui::Button("Flip board")) {
    flipBoard();
  }

  static int showMovesVal{ static_cast<int>(showMoves_) };
  static const char* showMovesNames[] = { "None", "Engine-only", "All" };
  const char* showMovesCurrentName    = showMovesNames[showMovesVal];
  if (ImGui::SliderInt("Show moves", &showMovesVal, 0,
                       static_cast<int>(ShowMoves::Count) - 1,
                       showMovesCurrentName, ImGuiSliderFlags_NoInput)) {
    showMoves_ = static_cast<ShowMoves>(showMovesVal);
  }

  ImGui::InputFloat("Engine max time, s", &engineMaxTimeSeconds_, 0.1F,
                    0.5F);
  ImGui::Text("Engine timer:");
  ImGui::SameLine();
  ImGui::ProgressBar(engineTimer_ / engineMaxTimeSeconds_);

  ImGui::End();

  // ImGui::ShowDemoWindow();

  rlImGuiEnd();
}

void Game::drawAdvantageBar() const {

  using namespace ImGui;
  constexpr float scale = 600;
  float scoreFracWhite{ 0.5 };
  std::string scoreStr{ "X" };
  int scorePlayer{ bestScore_ *
                   (s_.state().playerToMove() == humanPlayer_ ? 1 : -1) };
  float scorePlayerNorm = Evaluator::normalize(scorePlayer);
  scoreFracWhite        = (scorePlayerNorm + 1) / 2;
  if (humanPlayer_ != Player::White) {
    scoreFracWhite = 1 - scoreFracWhite;
  }
  scoreStr = Evaluator::formatScoreNorm(scorePlayer);

  raylib::Rectangle barRect{ boardRect_.x + boardRect_.width,
                             boardRect_.y, s_AdvantageBarWidth,
                             boardRect_.height };
  const int fontSize{ 15 };
  DrawText(scoreStr.c_str(),
           barRect.x + (barRect.width / 2.F) -
               (MeasureText(scoreStr.c_str(), fontSize) / 2.F),
           barRect.y - fontSize - 5, fontSize, ::DARKGRAY);
  DrawRectangleRec(barRect, s_BlackColor);
  DrawRectangleLinesEx(barRect, 2, ::DARKGRAY);
  barRect.y += barRect.height * (1 - scoreFracWhite);
  barRect.height += barRect.height * scoreFracWhite;
  DrawRectangleRec(barRect, s_WhiteColor);
}

void Game::restartAs(Player player) {
  state_       = State::Play;
  humanPlayer_ = player;
  s_.reset();
  s_.startEngineSearch();
}

void Game::selectTile(Coord pos) {
  selectedTile_ = pos;
  drawMoves_.clear();
  std::ranges::copy_if(s_.availableMoves(),
                       std::back_inserter(drawMoves_),
                       [&](const auto& m) {
                         return m.from == pos;
                       });
  if (drawMoves_.empty()) {
    resetSelection();
  }
}

} // namespace kamisado
