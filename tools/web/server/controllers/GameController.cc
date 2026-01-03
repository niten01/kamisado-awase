#include "GameController.h"
#include "drogon/HttpResponse.h"
#include "drogon/HttpTypes.h"
#include "plugins/SessionManagerPlugin.h"
#include "utils/Utils.h"

namespace kamisado {

void GameController::move(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int sessionID) {
  try {
    auto& session =
        app().getPlugin<SessionManagerPlugin>()->get(sessionID);


    auto [authSessionID, authPlayer] =
        *app().getPlugin<SessionManagerPlugin>()->auth(parseToken(req));
    if (authSessionID != sessionID) {
      sendJsonError("Unauthorized", std::move(callback),
                    drogon::k401Unauthorized);
      return;
    }

    if (authPlayer != session.playerToMove()) {
      sendJsonError("Not your turn", std::move(callback));
      return;
    }

    auto body{ req->jsonObject() };
    if (!body) {
      sendJsonError("No json body", std::move(callback));
      return;
    }

    if (!body->isMember("from") || !body->isMember("to")) {
      sendJsonError("No from/to field", std::move(callback));
      return;
    }

    auto from = fileRankToCoord((*body)["from"].asString());
    auto to   = fileRankToCoord((*body)["to"].asString());
    if (!from || !to) {
      sendJsonError("Invalid from/to", std::move(callback));
      return;
    }

    Move move{ .from = *from, .to = *to };
    session.makeMove(move);
    callback(HttpResponse::newHttpResponse());
  } catch (const SessionException& e) {
    sendJsonError(e.what(), std::move(callback));
    return;
  }
}

void GameController::state(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int sessionID) {
  Session* session = nullptr;
  try {
    session = &app().getPlugin<SessionManagerPlugin>()->get(sessionID);
  } catch (const SessionException& e) {
    sendJsonError(e.what(), std::move(callback));
    return;
  }

  callback(HttpResponse::newHttpJsonResponse(session->stateJson()));
}

} // namespace kamisado
