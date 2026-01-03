#pragma once
#include "kamisado/GameService.hpp"
#include "kamisado/GameState.hpp"
#include "kamisado/SearchEngine.hpp"
#include <raylib-cpp.hpp>
#include <raylib.h>
#include <unordered_set>

namespace kamisado {

class Game {
public:
  Game();
  ~Game();

  void run();

private:
  void updateLogic();
  void updateHumanMove();
  void updateEngineMove();
  void draw();
  void drawBoard();
  void drawTowers();
  void drawMove(Move m, raylib::Color c);
  [[nodiscard]] auto testHitCoord(raylib::Vector2 pos) const
      -> std::optional<Coord>;
  void drawGUI();
  void resetSelection();
  void selectTile(Coord pos);
  void makeMove(Move m);

  void flipBoard();
  void drawAdvantageBar() const;

  void restartAs(Player player);

private:
  static constexpr int s_WindowSize        = 800;
  static constexpr int s_BoardSize         = 600;
  static constexpr int s_AdvantageBarWidth = 30;
  // NOLINTBEGIN
  static inline const raylib::Color s_MoveColor{ 0x00000055 };
  static inline const raylib::Color s_WhiteColor{ ::WHITE };
  static inline const raylib::Color s_BlackColor{ ::BLACK };
  // NOLINTEND

  enum class State : uint8_t {
    Play,
    GameOver
  };

  raylib::Window window_;
  State state_{ State::Play };

  bool flipped_{ false };
  Player humanPlayer_{ Player::White };
  GameService s_;

  Rectangle boardRect_{};
  std::array<std::array<Rectangle, config::BoardSize>, config::BoardSize>
      tiles_{};
  std::optional<Coord> selectedTile_;
  std::vector<Move> drawMoves_;
  std::optional<Move> bestMove_;
  int bestScore_{ 0 };

  enum class ShowMoves : uint8_t {
    None,
    Engine,
    All,
    Count
  };
  ShowMoves showMoves_{ ShowMoves::Engine };
  float engineMaxTimeSeconds_{ 5.F };
  float engineTimer_{ 0.F };
};

} // namespace kamisado
