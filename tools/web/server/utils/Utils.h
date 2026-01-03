#include "drogon/HttpTypes.h"
#include "plugins/SessionManagerPlugin.h"
#include <drogon/HttpController.h>

namespace kamisado {

void sendJsonError(
    const std::string& message,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    drogon::HttpStatusCode status = drogon::k400BadRequest);

auto parseToken(const drogon::HttpRequestPtr& req) -> Token;

auto coordToFileRank(Coord c) -> std::string;

auto fileRankToCoord(const std::string& fileRank) -> std::optional<Coord>;

} // namespace kamisado
