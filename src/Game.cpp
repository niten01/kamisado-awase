#include "kamisado/Game.hpp"
#include <raylib.h>
#include <raymath.h>

namespace kamisado {

Game::Game()
    : window_{ s_WindowSize, s_WindowSize, "Mu-Torere" } {
  SetTargetFPS(60);
}

void Game::run() {
  while (!window_.ShouldClose()) {
    updateLogic();
    draw();
  }
}

void Game::updateLogic() {
}

void Game::draw() {
}


} // namespace kamisado
