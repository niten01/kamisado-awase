#pragma once
#include <cstdint>

namespace kamisado {

enum class Player : uint8_t {
  White,
  Black,
  Count
};

constexpr auto opposite(Player player) -> Player {
  return player == Player::White ? Player::Black : Player::White;
}

} // namespace kamisado
