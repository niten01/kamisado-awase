#pragma once
#include "kamisado/Evaluator.hpp"
#include "kamisado/GameState.hpp"
#include "kamisado/Move.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <array>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

namespace kamisado {

class SearchEngine {
  enum class Bound : uint8_t {
    Exact,
    Lower,
    Upper
  };

  struct TTEntry {
    uint64_t key{ 0 };
    int depthRemaining{ 0 };
    int score{ 0 };
    Bound bound{ Bound::Exact };
    Move bestMove{};
    bool hasBest{ false };
  };

public:
  struct Result {
    std::optional<Move> bestMove;
    int score{ 0 };
  };

  explicit SearchEngine(size_t ttSizePow2 = 1U << 20U);
  ~SearchEngine();

  void reset();
  [[nodiscard]] auto nodes() const -> uint64_t;

  void setCallback(std::function<void(const Result&)> callback);
  void startSearch(const GameState& s, int maxDepth);

  void stopSearch();

  [[nodiscard]] auto running() -> bool;
  [[nodiscard]] auto currentBest() const -> std::optional<Result>;

private:
  auto probe(uint64_t key) -> TTEntry*;

  void store(uint64_t key, int depthRemainig, int score, Bound bound,
             std::optional<Move> bestMove);

  static auto moveOrderingScore(const GameState& s, const Move& move)
      -> int;

  auto searchRoot(const GameState& s, int depth, int alpha, int beta,
                  std::optional<Move> pvHint = std::nullopt) -> Result;

  auto alphaBeta(const GameState& s, int depth, int alpha, int beta,
                 int ply, Player perspective) -> int;

  auto negamaxLoop(const GameState& s, const std::vector<Move>& moves,
                   Player perspective, int depth, int alpha, int beta,
                   int ply) -> Result;

private:
  static constexpr int s_Inf{ std::numeric_limits<int>::max() / 1000 *
                              1000 };

  std::vector<TTEntry> tt_;
  uint64_t nodes_{ 0 };
  int depth_{ 0 };
  int targeDepth_{ 0 };
  std::function<void(const Result&)> resultCallback_{ [](auto&&) {
  } };
  std::optional<Result> currentBest_;
  std::array<std::array<std::optional<Move>, 2>, 128> killers_;
  std::optional<Move> pv_;
  std::thread searchThread_;
  std::atomic<bool> running_{ false };
};

} // namespace kamisado
