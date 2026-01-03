#pragma once

#include "kamisado/Player.hpp"
#include <drogon/WebSocketController.h>

using namespace drogon;

namespace kamisado {

struct ConnContext {
  int sessionID;
  Player side;
};

class WsSessionController
    : public drogon::WebSocketController<WsSessionController> {
public:
  void handleNewMessage(const WebSocketConnectionPtr&, std::string&&,
                        const WebSocketMessageType&) override;
  void handleNewConnection(const HttpRequestPtr&,
                           const WebSocketConnectionPtr&) override;
  void handleConnectionClosed(const WebSocketConnectionPtr&) override;
  WS_PATH_LIST_BEGIN
  // list path definitions here;
  WS_PATH_ADD("/ws/sessions", "kamisado::AuthFilter");
  WS_PATH_LIST_END
};

} // namespace kamisado
