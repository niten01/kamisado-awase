#include "kamisado/GameState.hpp"
#include "kamisado/BoardProps.hpp"
#include "kamisado/Config.hpp"
#include <cstdint>

namespace kamisado {

namespace {
// Source - https://stackoverflow.com/a
// Posted by Scott Howlett, modified by community. See post 'Timeline' for
// change history Retrieved 2025-12-27, License - CC BY-SA 3.0
template <class T>
inline void hash_combine(uint64_t& seed, const T& v) {
  std::hash<T> hasher;
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a          = (hasher(v) ^ seed) * kMul;
  a ^= (a >> 47U);
  uint64_t b = (seed ^ a) * kMul;
  b ^= (b >> 47U);
  seed = b * kMul;
}

class Zobrist {
public:
  Zobrist() {
    uint64_t x = 0x9E3779B97F4A7C15ULL;
    auto next  = [&]() {
      // SplitMix64
      x += 0x9E3779B97F4A7C15ULL;
      uint64_t z = x;
      z          = (z ^ (z >> 30U)) * 0xBF58476D1CE4E5B9ULL;
      z          = (z ^ (z >> 27U)) * 0x94D049BB133111EBULL;
      return z ^ (z >> 31U);
    };

    for (auto& sq : piece_) {
      for (auto& pt : sq) {
        pt = next();
      }
    }
    stm_ = next();
    for (auto& f : forced_) {
      f = next();
    }
  }

  [[nodiscard]] auto towerKey(Coord pos, Tower tower) const -> uint64_t {
    auto posIdx   = (pos.row * config::BoardSize) + pos.col;
    auto towerIdx = (static_cast<size_t>(Player::Count) *
                     static_cast<size_t>(tower.owner)) +
                    static_cast<size_t>(tower.color);
    return piece_[posIdx][towerIdx];
  }

  [[nodiscard]] auto forcedKey(std::optional<Color> forcedColor) const
      -> uint64_t {
    return forced_[static_cast<size_t>(
        forcedColor.value_or(Color::Count))];
  }

  static auto instance() -> Zobrist& {
    static Zobrist zobrist;
    return zobrist;
  }

private:
  std::array<std::array<uint64_t, config::BoardSize * 2>,
             config::BoardSize * config::BoardSize>
      piece_{};
  uint64_t stm_{};
  std::array<uint64_t, static_cast<size_t>(Color::Count) + 1ULL>
      forced_{};
};

} // namespace

GameState::GameState(Board board)
    : board_{ board },
      goals_{ board.coloring() } {
  recalculateHash();
}

auto GameState::board() const -> const Board& {
  return board_;
}

auto GameState::playerToMove() const -> Player {
  return playerToMove_;
}

auto GameState::forcedColor() const -> std::optional<Color> {
  return forcedColor_;
}

void GameState::recalculateHash() {
  hash_ = std::hash<Player>{}(playerToMove_);
  hash_combine(hash_, Zobrist::instance().forcedKey(forcedColor_));

  for (int r = 0; r < static_cast<int>(board_.size()); r++) {
    for (int c = 0; c < static_cast<int>(board_.size()); c++) {
      auto tower{ board_.towerAt(Coord{ r, c }) };
      if (!tower) {
        continue;
      }

      hash_combine(hash_,
                   Zobrist::instance().towerKey(Coord{ r, c }, *tower));
    }
  }
}

auto GameState::terminalStatus() const -> Outcome {
  Outcome o;

  for (int color = 0; color < static_cast<int>(Color::Count); color++) {
    Color towerColor{ static_cast<Color>(color) };

    for (int p = 0; p < static_cast<int>(Player::Count); p++) {
      Player player{ static_cast<Player>(p) };
      Coord pos{ board_.towerPos(player, towerColor) };

      if (pos == goals_.goal(player, towerColor)) {
        o.winner   = player;
        o.terminal = true;
        return o;
      }
    }
  }

  int repeats{ 0 };
  for (auto h : history_) {
    if (h == hash_) {
      repeats++;
      if (repeats >= static_cast<int>(Player::Count)) {
        o.terminal = true;
        o.winner   = playerToMove_;
        return o;
      }
    }
  }

  return o;
}

auto GameState::apply(Move move) const -> GameState {
  auto gs = *this;
  gs.applyInPlace(move);
  return gs;
}

void GameState::applyInPlace(Move move) {
  if (move.isPass) {
    forcedColor_ = board_.coloring().at(move.from);
  } else {
    auto towerToMove = board_.towerAt(move.from);
    assert(towerToMove && "Invalid move (no tower)");
    assert(towerToMove->owner == playerToMove_ &&
           "Invalid move (not own tower)");

    board_.move(*towerToMove, move.from, move.to);
    forcedColor_ = board_.coloring().at(move.to);
  }

  playerToMove_ = opposite(playerToMove_);
  auto oldHash  = hash_;
  recalculateHash();
  history_.push_back(oldHash);
}

auto GameState::hash() const -> uint64_t {
  return hash_;
}

auto GameState::goals() const -> Goals {
  return goals_;
}

} // namespace kamisado
