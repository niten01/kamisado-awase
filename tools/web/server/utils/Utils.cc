#include "Utils.h"
#include "kamisado/Config.hpp"

namespace kamisado {

// NOLINTNEXTLINE
const std::unordered_set<std::string> allowedOrigins = {
  "https://niten01.github.io",
  "http://localhost:5173",
};

void sendJsonError(
    const std::string& message,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    drogon::HttpStatusCode status) {
  Json::Value ret;
  ret["message"] = message;
  auto resp      = drogon::HttpResponse::newHttpJsonResponse(ret);
  resp->setStatusCode(status);
  std::move(callback)(resp);
}

auto parseToken(const drogon::HttpRequestPtr& req) -> Token {
  auto token = req->getHeader("Authorization");
  if (token.empty()) {
    token = req->getParameter("token");
    if (token.empty()) {
      return {};
    }
  } else {
    token = token.substr(sizeof("Bearer ") - 1);
  }

  return token;
}

namespace {
constexpr std::string_view files = "abcdefgh";
} // namespace

/// Coords are 0-indexed, rows start from top, while ranks start from
/// bottom
auto coordToFileRank(Coord c) -> std::string {
  return files[c.col] + std::to_string(config::BoardSize - c.row);
}

auto fileRankToCoord(const std::string& fileRank)
    -> std::optional<Coord> {
  if (fileRank.size() != 2) {
    return std::nullopt;
  }
  if (!std::isalpha(fileRank[0]) || !std::isdigit(fileRank[1])) {
    return std::nullopt;
  }

  // NOLINTNEXTLINE
  if (files.find(fileRank[0]) == std::string::npos) {
    return std::nullopt;
  }

  return Coord{ static_cast<uint8_t>(config::BoardSize -
                                     (fileRank[1] - '0')),
                static_cast<uint8_t>(fileRank[0] - 'a') };
}

} // namespace kamisado
