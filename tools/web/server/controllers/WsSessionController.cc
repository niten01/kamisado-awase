#include "WsSessionController.h"
#include "drogon/HttpAppFramework.h"
#include "plugins/SessionManagerPlugin.h"
#include "trantor/utils/Logger.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace kamisado {

void WsSessionController::handleNewMessage(
    const WebSocketConnectionPtr& wsConnPtr, std::string&& message,
    const WebSocketMessageType& type) {
}

void WsSessionController::handleNewConnection(
    const HttpRequestPtr& req, const WebSocketConnectionPtr& wsConnPtr) {
  try {
    int sessionID = std::stoi(req->getParameter("sid"));

    auto authRes = app().getPlugin<SessionManagerPlugin>()->auth(
        req->getParameter("token"));
    assert(authRes && "Auth filter failed");
    auto [authSessionID, authPlayer] = *authRes;
    if (authSessionID != sessionID) {
      return;
    }
    wsConnPtr->setContext(
        std::make_shared<ConnContext>(sessionID, authPlayer));

    app().getPlugin<SessionManagerPlugin>()->get(sessionID).subscribe(
        wsConnPtr);
    LOG_DEBUG << fmt::format("{} subscribed to session {}",
                             wsConnPtr->peerAddr().toIpPort(), sessionID);
  } catch (const std::invalid_argument& e) {
    LOG_WARN << "Could not parse session ID from query: " << req->query();
  } catch (const SessionException& e) {
    LOG_WARN << e.what();
  }
}

void WsSessionController::handleConnectionClosed(
    const WebSocketConnectionPtr& wsConnPtr) {
  try {
    auto ctx = wsConnPtr->getContext<ConnContext>();
    if (!ctx) {
      return;
    }
    auto* manager = app().getPlugin<SessionManagerPlugin>();
    manager->get(ctx->sessionID).unsubscribe(wsConnPtr);
    manager->leave(ctx->sessionID, ctx->side);
  } catch (const SessionException& e) {
    LOG_WARN << "Error on disconnect: " << e.what();
  }
}

} // namespace kamisado
