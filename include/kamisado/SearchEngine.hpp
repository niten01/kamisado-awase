#pragma once
#include "kamisado/GameState.hpp"
#include "kamisado/Move.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <optional>
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
    int depth{ 0 };
    int score{ 0 };
    Bound bound{ Bound::Exact };
    Move bestMove{};
    bool hasBest{ false };
  };

public:
  struct Result {
    Move bestMove{};
    bool hasMove{ false };
    int score{ 0 };
  };

  explicit SearchEngine(size_t ttSizePow2 = 1U << 20U);

  void resetStats();
  [[nodiscard]] auto nodes() const -> uint64_t;

  auto findBestMove(const GameState& s, int depth) -> Result {
    resetStats();
    Result r;
    r.score    = searchRoot(s, depth, -s_Inf, s_Inf).score;
    r.bestMove = pv_.value_or({});
    r.hasMove  = pv_.has_value();
    return r;
  }

private:
  auto probe(uint64_t key) -> TTEntry*;

  void store(uint64_t key, int depth, int score, Bound bound,
             std::optional<Move> bestMove);

  auto moveOrderingScore(const GameState& s, const Move& move) -> int;

  auto searchRoot(const GameState& s, int depth, int alpha, int beta,
                  std::optional<Move> pvHint = std::nullopt) -> Result;

  auto alphaBeta(const GameState& s, int depth, int alpha, int beta,
                 int ply, Player perspective) -> int;

private:
  static constexpr int s_Inf{ std::numeric_limits<int>::max() / 1000 *
                              1000 };

  std::vector<TTEntry> tt_;
  uint64_t nodes_{ 0 };
  std::array<std::array<std::optional<Move>, 2>, 128> killers_;
  std::optional<Move> pv_;
};

} // namespace kamisado
