#pragma once
#include "kamisado/Board.hpp"
#include "kamisado/BoardProps.hpp"
#include "kamisado/Move.hpp"
#include "kamisado/Outcome.hpp"
#include <json/value.h>

namespace kamisado {

auto toJson(Coord c) -> Json::Value;

auto toJson(Player player) -> Json::Value;

auto toJson(const Move& move) -> Json::Value;

auto toJson(const Outcome& outcome) -> Json::Value;

auto toJson(Color color) -> Json::Value;

auto toJson(const Tower& tower) -> Json::Value;

auto toJson(const Board& board) -> Json::Value;

} // namespace kamisado
