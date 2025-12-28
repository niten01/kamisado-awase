#include "kamisado/SearchEngine.hpp"
#include "kamisado/Evaluator.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <algorithm>
#include <ranges>

namespace kamisado {

SearchEngine::SearchEngine(size_t ttSizePow2)
    : tt_(ttSizePow2) {
  assert((ttSizePow2 & (ttSizePow2 - 1U)) == 0 &&
         "ttSizePow2 must be a power of 2");
}

auto SearchEngine::probe(uint64_t key) -> TTEntry* {
  TTEntry& e = tt_[key & (tt_.size() - 1)];
  if (e.key == key) {
    return &e;
  }
  return nullptr;
}

void SearchEngine::store(uint64_t key, int depth, int score, Bound bound,
                         std::optional<Move> bestMove) {
  TTEntry& e = tt_[key & (tt_.size() - 1)];

  if (e.key == 0 || depth >= e.depth) {
    e.key      = key;
    e.depth    = depth;
    e.score    = score;
    e.bound    = bound;
    e.bestMove = bestMove.value_or({});
    e.hasBest  = bestMove.has_value();
  }
}

auto SearchEngine::moveOrderingScore(const GameState& s, const Move& move)
    -> int {
  if (move.isPass) {
    return -100000;
  }

  const Player p = s.playerToMove();
  const Board& b = s.board();

  if (p == Player::Black && move.to.row == Board::WhiteHomeRow) {
    return 900000;
  }
  if (p == Player::White && move.to.row == Board::BlackHomeRow) {
    return 900000;
  }

  int advanceGain{ std::abs(move.to.row - move.from.row) };

  const Color forcedOpp{ b.coloring().at(move.to) };
  const int oppMob{ MoveGen::towerMobility(s.board(), opposite(p),
                                           forcedOpp) };

  return (1000 * advanceGain) - (50 * oppMob);
}

auto SearchEngine::searchRoot(const GameState& s, int depth, int alpha,
                              int beta, std::optional<Move> pvHint)
    -> Result {
  Result out;
  auto moves{ MoveGen::legalMoves(s) };
  if (moves.empty()) {
    out.score = Evaluator::evaluate(s, s.playerToMove());
    return out;
  }

  auto* tte{ probe(s.hash()) };
  if (tte && tte->hasBest) {
    auto it{ std::ranges::find_if(moves, [&](const auto& m) {
      return m == tte->bestMove;
    }) };
    if (it != moves.end()) {
      std::iter_swap(it, moves.begin());
    }
  }
  if (pvHint.has_value()) {
    auto it{ std::ranges::find_if(moves, [&](const auto& m) {
      return m == *pvHint;
    }) };
    if (it != moves.end()) {
      std::iter_swap(it, moves.begin());
    }
  }

  std::stable_sort(moves.begin() + 1, moves.end(),
                   [&](const auto& m1, const auto& m2) {
                     return moveOrderingScore(s, m1) >
                            moveOrderingScore(s, m2);
                   });

  const Player perspective{ s.playerToMove() };

  int bestScore{ -s_Inf };
  std::optional<Move> bestMove{};

  for (auto&& [i, move] : std::views::enumerate(moves)) {
    GameState child{ s.apply(move) };

    int score{};
    if (i == 0) {
      score = -alphaBeta(child, depth - 1, -beta, -alpha, 1,
                         opposite(perspective));
    } else {
      score = -alphaBeta(child, depth - 1, -(alpha + 1), -alpha, 1,
                         opposite(perspective));
      if (score > alpha && score < beta) {
        score = -alphaBeta(child, depth - 1, -beta, -alpha, 1,
                           opposite(perspective));
      }
    }

    if (score > bestScore) {
      bestScore = score;
      bestMove  = move;
    }
    alpha = std::max(alpha, score);

    if (alpha >= beta) {
      break;
    }
  }

  pv_          = bestMove;
  out.score    = bestScore;
  out.hasMove  = bestMove.has_value();
  out.bestMove = bestMove.value_or({});
  return out;
}

auto SearchEngine::alphaBeta(const GameState& s, int depth, int alpha,
                             int beta, int ply, Player perspective)
    -> int {
  ++nodes_;

  if (depth <= 0) {
    return Evaluator::evaluate(s, perspective);
  }

  auto status{ s.terminalStatus() };
  if (status.terminal) {
    return Evaluator::evaluate(s, perspective);
  }

  auto* tte{ probe(s.hash()) };
  if (tte && tte->depth >= depth) {
    if (tte->bound == Bound::Exact) {
      return tte->score;
    }
    if (tte->bound == Bound::Lower) {
      alpha = std::max(alpha, tte->score);
    }
    if (tte->bound == Bound::Upper) {
      beta = std::min(beta, tte->score);
    }
    if (alpha >= beta) {
      return tte->score;
    }
  }

  auto moves{ MoveGen::legalMoves(s) };
  if (moves.empty()) {
    return Evaluator::evaluate(s, perspective);
  }

  if (tte && tte->hasBest) {
    auto it{ std::ranges::find_if(moves, [&](const auto& m) {
      return m == tte->bestMove;
    }) };
    if (it != moves.end()) {
      std::iter_swap(it, moves.begin());
    }
  }

  if (ply < static_cast<int>(killers_.size())) {
    for (auto k : killers_[ply]) {
      if (!k) {
        continue;
      }
      auto it{ std::ranges::find_if(moves, [&](const auto& m) {
        return m == *k;
      }) };
      if (it != moves.end()) {
        std::iter_swap(it, moves.begin());
      }
    }
  }

  std::ranges::stable_sort(moves, [&](const auto& m1, const auto& m2) {
    return moveOrderingScore(s, m1) > moveOrderingScore(s, m2);
  });

  int bestScore{ -s_Inf };
  std::optional<Move> bestMove{};
  const int origAlpha{ alpha };

  for (auto&& [i, move] : std::views::enumerate(moves)) {
    GameState child{ s.apply(move) };

    int score{};
    if (i == 0) {
      score = -alphaBeta(child, depth - 1, -beta, -alpha, ply + 1,
                         opposite(perspective));
    } else {
      score = -alphaBeta(child, depth - 1, -(alpha + 1), -alpha, ply + 1,
                         opposite(perspective));
      if (score > alpha && score < beta) {
        score = -alphaBeta(child, depth - 1, -beta, -alpha, ply + 1,
                           opposite(perspective));
      }
    }

    if (score > bestScore) {
      bestScore = score;
      bestMove  = move;
    }
    alpha = std::max(alpha, score);

    if (alpha >= beta) {
      if (!move.isPass && ply < static_cast<int>(killers_.size())) {
        if (!killers_[ply][0] || *killers_[ply][0] != move) {
          killers_[ply][1] = killers_[ply][0];
          killers_[ply][0] = move;
        }
      }
    }
  }

  Bound bound{ Bound::Exact };
  if (bestScore <= origAlpha) {
    bound = Bound::Upper;
  } else if (bestScore >= beta) {
    bound = Bound::Lower;
  }
  store(s.hash(), depth, bestScore, bound, bestMove);
  return bestScore;
}

void SearchEngine::resetStats() {
  nodes_ = 0;
}
auto SearchEngine::nodes() const -> uint64_t {
  return nodes_;
}
} // namespace kamisado
