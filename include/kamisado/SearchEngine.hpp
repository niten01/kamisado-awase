#pragma once
#include "kamisado/Evaluator.hpp"
#include "kamisado/GameState.hpp"
#include "kamisado/Move.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <thread>
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

  void resetStats();
  [[nodiscard]] auto nodes() const -> uint64_t;

  void startSearch(const GameState& s, int maxDepth) {
    resetStats();
    running_      = true;
    searchThread_ = std::jthread([this, maxDepth, s]() {
      for (int d = 1; d <= maxDepth && running_; d++) {
        int window{ 50 };
        int alpha{ -s_Inf };
        int beta{ s_Inf };
        if (d > 1) {
          alpha = currentBest_->score - window;
          beta  = currentBest_->score + window;
        }
        std::optional<Move> pvHint{ currentBest_.has_value()
                                        ? currentBest_->bestMove
                                        : std::nullopt };

        auto r{ searchRoot(s, d, alpha, beta, pvHint) };

        if (r.score <= alpha || r.score >= beta) {
          r = searchRoot(s, d, -s_Inf, s_Inf, pvHint);
        }

        std::cout << fmt::format("Depth: {}; bestScore: {}; move: {}\n",
                                 d, r.score, r.bestMove.value_or({}));
        if (r.bestMove) {
          currentBest_ = r;
        } else {
          break;
        }

        if (Evaluator::isMateScore(r.score)) {
          break;
        }
      }
    });
  }

  void stopSearch() {
    running_ = false;
    searchThread_.join();
  }

  [[nodiscard]] auto running() const -> bool {
    return running_;
  }
  [[nodiscard]] auto currentBest() const -> std::optional<Result> {
    return currentBest_;
  }

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
  std::optional<Result> currentBest_;
  std::array<std::array<std::optional<Move>, 2>, 128> killers_;
  std::optional<Move> pv_;
  std::jthread searchThread_;
  std::atomic<bool> running_{ false };
};

} // namespace kamisado
