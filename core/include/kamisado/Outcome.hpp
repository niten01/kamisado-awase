#pragma once
#include "kamisado/Player.hpp"
#include <optional>

namespace kamisado {

struct Outcome {
  bool terminal{ false };
  std::optional<Player> winner;
};

} // namespace kamisado
