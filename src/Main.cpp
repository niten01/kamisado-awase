#include "kamisado/Game.hpp"
#include "kamisado/GameState.hpp"
#include "kamisado/SearchEngine.hpp"

auto main() -> int {
  // kamisado::GameState gs{kamisado::Board{kamisado::BoardColoring::official()}};
  // kamisado::SearchEngine engine;
  // engine.startSearch(gs, 5);
  // while(engine.running()){
  //   std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // }
  // return 0;

  kamisado::Game game;
  game.run();
}
