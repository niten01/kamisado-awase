#pragma once
#include "kamisado/GameState.hpp"
#include "kamisado/SearchEngine.hpp"
#include <memory>
#include <raylib-cpp.hpp>
#include <raylib.h>
#include <unordered_set>

namespace kamisado {

class Game {
public:
  Game();

  void run();

private:
  void updateLogic();
  void updateHumanMove();
  void updateEngineMove();
  void draw();
  void drawBoard();
  void drawMove(Move m, raylib::Color c);
  [[nodiscard]] auto testHitCoord(raylib::Vector2 pos) const
      -> std::optional<Coord>;
  void resetSelection();
  void makeMove(Move m);

private:
  static constexpr int s_EngineDepth = 4;

  static constexpr int s_WindowSize = 800;
  static constexpr int s_BoardSize  = 600;
  // NOLINTNEXTLINE
  static inline const raylib::Color s_MoveColor{ 0x00000055 };

  enum class State : uint8_t {
    ChooseSide,
    Play,
    GameOver
  };

  struct CoordHasher {
    constexpr auto operator()(const Coord& c) const -> std::size_t {
      return std::hash<int>()(c.row) ^ std::hash<int>()(c.col);
    }
  };

  raylib::Window window_;
  State state_{ State::Play };

  int turn_{ 1 };
  GameState gameState_;
  Player humanPlayer_{ Player::White };
  SearchEngine engine_;
  std::vector<Move> availableMoves_;
  std::unordered_set<Coord, CoordHasher> canMoveFrom_;

  Rectangle boardRect_{};
  std::array<std::array<Rectangle, config::BoardSize>, config::BoardSize>
      tiles_{};
  std::optional<Coord> selectedTile_;
  std::vector<Move> drawMoves_;
  Move lastMove_;
};

} // namespace kamisado
