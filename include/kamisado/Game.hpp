#pragma once
#include <memory>
#include <raylib-cpp.hpp>
#include <raylib.h>

namespace kamisado {

class Game {
public:
  Game();

  void run();

private:
  void updateLogic();
  void draw();

private:
  static constexpr int s_WindowSize   = 800;

  enum class State : uint8_t {
    ChooseSide,
    Play,
    GameOver
  };

  raylib::Window window_;
  float AITimer_{};
  State state_{ State::Play };
};

} // namespace kamisado
