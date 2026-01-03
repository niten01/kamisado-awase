#include "SessionsController.h"
#include "drogon/HttpTypes.h"
#include "plugins/SessionManagerPlugin.h"
#include "utils/Utils.h"

namespace kamisado {

void SessionsController::create(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto body{ req->jsonObject() };
  if (!body) {
    sendJsonError("No json body", std::move(callback));
    return;
  }

  if (!body->isMember("analysisEnabled")) {
    sendJsonError("No analysisEnabled field", std::move(callback));
    return;
  }

  SessionOptions options;
  options.analysisEnabled = (*body)["analysisEnabled"].asBool();
  int sessionId =
      app().getPlugin<SessionManagerPlugin>()->create(options);
  Json::Value respBody;
  respBody["sessionId"] = sessionId;
  callback(HttpResponse::newHttpJsonResponse(std::move(respBody)));
}

void SessionsController::join(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int sessionID) {
  try {
    auto body{ req->jsonObject() };
    auto* manager = app().getPlugin<SessionManagerPlugin>();
    auto& session = manager->get(sessionID);

    Player sidePlayer{};

    if (body && body->isMember("side")) {
      auto side = (*body)["side"].asString();
      if (side != "white" && side != "black") {
        sendJsonError("Invalid side", std::move(callback));
        return;
      }
      if (side == "black") {
        sidePlayer = Player::Black;
      } else {
        sidePlayer = Player::White;
      }
    } else {
      sidePlayer = manager->randomFreeSlot(sessionID);
    }

    auto token = manager->join(sessionID, sidePlayer);

    Json::Value respBody;
    respBody["token"]           = token;
    respBody["analysisEnabled"] = session.analysisEnabled();
    respBody["side"] = sidePlayer == Player::White ? "white" : "black";
    callback(HttpResponse::newHttpJsonResponse(std::move(respBody)));
  } catch (const SessionException& e) {
    sendJsonError(fmt::format("Internal error: {}", e.what()),
                  std::move(callback), drogon::k500InternalServerError);
    return;
  }
}

} // namespace kamisado
