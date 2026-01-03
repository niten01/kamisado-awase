#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

namespace kamisado {

class GameController : public drogon::HttpController<GameController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(GameController::move, "/api/sessions/{1}/move", Post,
                "kamisado::AuthFilter");
  ADD_METHOD_TO(GameController::state, "/api/sessions/{1}/state", Get,
                "kamisado::AuthFilter");
  METHOD_LIST_END

  void move(const HttpRequestPtr& req,
            std::function<void(const HttpResponsePtr&)>&& callback,
            int sessionID);

  void state(const HttpRequestPtr& req,
             std::function<void(const HttpResponsePtr&)>&& callback,
             int sessionID);
};

} // namespace kamisado
