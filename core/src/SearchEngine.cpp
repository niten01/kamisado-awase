#include "kamisado/SearchEngine.hpp"
#include "kamisado/Evaluator.hpp"
#include "kamisado/MoveGen.hpp"
#include "kamisado/Player.hpp"
#include <algorithm>
#include <iostream>
#include <ranges>

namespace kamisado {

SearchEngine::SearchEngine(size_t ttSizePow2)
    : tt_(ttSizePow2) {
  assert((ttSizePow2 & (ttSizePow2 - 1U)) == 0 &&
         "ttSizePow2 must be a power of 2");
}

SearchEngine::~SearchEngine() {
  stopSearch();
}

auto SearchEngine::probe(uint64_t key) -> TTEntry* {
  TTEntry& e = tt_[key & (tt_.size() - 1)];
  if (e.key == key) {
    return &e;
  }
  return nullptr;
}

void SearchEngine::store(uint64_t key, int depthRemainig, int score,
                         Bound bound, std::optional<Move> bestMove) {
  TTEntry& e = tt_[key & (tt_.size() - 1)];

  if (e.key == 0 || depthRemainig >= e.depthRemaining) {
    e.key            = key;
    e.depthRemaining = depthRemainig;
    e.score          = score;
    e.bound          = bound;
    e.bestMove       = bestMove.value_or({});
    e.hasBest        = bestMove.has_value();
  }
}

auto SearchEngine::moveOrderingScore(const GameState& s, const Move& move)
    -> int {
  if (move.isPass) {
    return -100000;
  }

  const Player p = s.playerToMove();
  const Board& b = s.board();
  assert(b.towerAt(move.from).has_value() && "No tower to move");
  const Tower towerToMove = *b.towerAt(move.from);
  assert(towerToMove.owner == p && "Not own tower");

  if (move.to.row == s.goals().row(p) &&
      move.to.col == s.goals().col(p, towerToMove.color)) {
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
  auto moves{ MoveGen::legalMoves(s) };
  if (moves.empty()) {
    Result out;
    out.score = Evaluator::evaluate(s, s.playerToMove());
    return out;
  }

  bool firstGood{ false };
  auto* tte{ probe(s.hash()) };
  if (tte && tte->hasBest) {
    auto it{ std::ranges::find_if(moves, [&](const auto& m) {
      return m == tte->bestMove;
    }) };
    if (it != moves.end()) {
      std::iter_swap(it, moves.begin());
      firstGood = true;
    }
  }
  if (pvHint.has_value()) {
    auto it{ std::ranges::find_if(moves, [&](const auto& m) {
      return m == *pvHint;
    }) };
    if (it != moves.end()) {
      std::iter_swap(it, moves.begin());
      firstGood = true;
    }
  }

  std::stable_sort(moves.begin() + (firstGood ? 1 : 0), moves.end(),
                   [&](const auto& m1, const auto& m2) {
                     return moveOrderingScore(s, m1) >
                            moveOrderingScore(s, m2);
                   });

  const Player perspective{ s.playerToMove() };

  Result result{ negamaxLoop(s, moves, perspective, depth, alpha, beta,
                             1) };

  pv_ = result.bestMove;
  return result;
}

auto SearchEngine::alphaBeta(const GameState& s, int depth, int alpha,
                             int beta, int ply, Player perspective)
    -> int {
  ++nodes_;

  auto status{ s.terminalStatus() };
  if (status.terminal) {
    return Evaluator::mateScore(status.winner == perspective, ply);
  }

  if (depth <= 0 || !running_) {
    return Evaluator::evaluate(s, perspective);
  }

  auto* tte{ probe(s.hash()) };
  if (tte && tte->depthRemaining >= depth) {
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

  bool firstGood{ false };
  if (tte && tte->hasBest) {
    auto it{ std::ranges::find_if(moves, [&](const auto& m) {
      return m == tte->bestMove;
    }) };
    if (it != moves.end()) {
      std::iter_swap(it, moves.begin());
      firstGood = true;
    }
  } else if (ply < static_cast<int>(killers_.size())) {
    for (auto k : killers_[ply]) {
      if (!k) {
        continue;
      }
      auto it{ std::ranges::find_if(moves, [&](const auto& m) {
        return m == *k;
      }) };
      if (it != moves.end()) {
        std::iter_swap(it, moves.begin());
        firstGood = true;
      }
    }
  }

  std::stable_sort(moves.begin() + (firstGood ? 1 : 0), moves.end(),
                   [&](const auto& m1, const auto& m2) {
                     return moveOrderingScore(s, m1) >
                            moveOrderingScore(s, m2);
                   });

  Result result{ negamaxLoop(s, moves, perspective, depth, alpha, beta,
                             ply) };

  Bound bound{ Bound::Exact };
  if (result.score <= alpha) {
    bound = Bound::Upper;
  } else if (result.score >= beta) {
    bound = Bound::Lower;
  }
  store(s.hash(), depth, result.score, bound, result.bestMove);
  return result.score;
}

void SearchEngine::reset() {
  stopSearch();
  nodes_ = 0;
  currentBest_.reset();
  depth_ = 0;
}
auto SearchEngine::nodes() const -> uint64_t {
  return nodes_;
}

auto SearchEngine::negamaxLoop(const GameState& s,
                               const std::vector<Move>& moves,
                               Player perspective, int depth, int alpha,
                               int beta, int ply) -> Result {
  int bestScore{ -s_Inf };
  std::optional<Move> bestMove{};
  for (auto&& [i, move] : std::views::enumerate(moves)) {

    GameState child{ s.apply(move) };

    int extDepth{ 0 };
    assert(child.forcedColor() && "Move 2+ should have forced color");
    int childMobility{ MoveGen::towerMobility(
        child.board(), child.playerToMove(), *child.forcedColor()) };
    if (childMobility <= 1) {
      extDepth = 1;
    }

    int score{};
    if (i == 0) {
      score = -alphaBeta(child, depth - 1 + extDepth, -beta, -alpha,
                         ply + 1, opposite(perspective));
    } else {
      score = -alphaBeta(child, depth - 1 + extDepth, -(alpha + 1),
                         -alpha, ply + 1, opposite(perspective));
      if (score > alpha && score < beta) {
        score = -alphaBeta(child, depth - 1 + extDepth, -beta, -alpha,
                           ply + 1, opposite(perspective));
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

  return {
    .bestMove = bestMove,
    .score    = bestScore,
  };
}

void SearchEngine::startSearch(const GameState& s, int maxDepth) {
  reset();
  targeDepth_   = maxDepth;
  running_      = true;
  searchThread_ = std::thread([this, s]() {
    for (depth_ = 1; depth_ <= targeDepth_ && running_; depth_++) {
      int window{ 50 };
      int alpha{ -s_Inf };
      int beta{ s_Inf };
      if (depth_ > 1) {
        alpha = currentBest_->score - window;
        beta  = currentBest_->score + window;
      }
      std::optional<Move> pvHint{ currentBest_.has_value()
                                      ? currentBest_->bestMove
                                      : std::nullopt };

      auto r{ searchRoot(s, depth_, alpha, beta, pvHint) };

      if (r.score <= alpha || r.score >= beta) {
        r = searchRoot(s, depth_, -s_Inf, s_Inf, pvHint);
      }

      if (r.bestMove && running_) {
        currentBest_ = r;
        resultCallback_(r);
        // std::cout << fmt::format("Depth: {}; bestScore: {}; move:
        // {}\n",
        //                          depth_,
        //                          Evaluator::formatScore(r.score),
        //                          r.bestMove.value_or({}));
      } else {
        break;
      }

      if (Evaluator::isMateScore(r.score)) {
        break;
      }
    }
  });
}

void SearchEngine::stopSearch() {
  if (running_.exchange(false)) {
    searchThread_.join();
  }
}

auto SearchEngine::running() -> bool {
  if (depth_ > targeDepth_) {
    stopSearch();
  }
  return running_;
}

auto SearchEngine::currentBest() const -> std::optional<Result> {
  return currentBest_;
}

void SearchEngine::setCallback(
    std::function<void(const Result&)> callback) {
  resultCallback_ = std::move(callback);
}

} // namespace kamisado
