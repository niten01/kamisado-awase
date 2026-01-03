#include "Json.h"
#include "Utils.h"

namespace kamisado {

auto toJson(Coord c) -> Json::Value {
  return coordToFileRank(c);
}

auto toJson(Player player) -> Json::Value {
  return player == Player::White ? "white" : "black";
}

auto toJson(const Move& move) -> Json::Value {
  Json::Value out;
  out["from"] = toJson(move.from);
  out["to"]   = toJson(move.to);
  return out;
}

auto toJson(const Outcome& outcome) -> Json::Value {
  Json::Value out;
  if (outcome.terminal) {
    out["status"] = "win";
    out["winner"] = toJson(*outcome.winner);
  } else {
    out["status"] = "ongoing";
  }
  return out;
}

auto toJson(Color color) -> Json::Value {
  switch (color) {
  case Color::Brown:
    return "brown";
  case Color::Green:
    return "green";
  case Color::Red:
    return "red";
  case Color::Yellow:
    return "yellow";
  case Color::Pink:
    return "pink";
  case Color::Purple:
    return "purple";
  case Color::Blue:
    return "blue";
  case Color::Orange:
    return "orange";
  default:
    assert(false && "Unknown color");
    return { Json::nullValue };
  }
}

auto toJson(const Tower& tower) -> Json::Value {
  Json::Value out;
  out["side"]  = toJson(tower.owner);
  out["color"] = toJson(tower.color);
  return out;
}

auto toJson(const Board& board) -> Json::Value {
  Json::Value out(Json::arrayValue);
  for (int r = 0; r < board.size(); r++) {
    Json::Value row(Json::arrayValue);
    for (int c = 0; c < board.size(); c++) {
      Coord pos{ r, c };
      Json::Value cell;
      cell["color"] = toJson(board.coloring().at(pos));
      if (auto tower = board.towerAt(pos)) {
        cell["piece"] = toJson(*tower);
      }
      // cell["dbg"] = fmt::format("({},{});{}", r, c, coordToFileRank(pos));
      row.append(cell);
    }
    out.append(row);
  }
  return out;
}

} // namespace kamisado
