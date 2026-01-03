#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

namespace kamisado {

class SessionsController
    : public drogon::HttpController<SessionsController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SessionsController::create, "/api/sessions", Post);
  ADD_METHOD_TO(SessionsController::join, "/api/sessions/{1}/join", Post);
  METHOD_LIST_END

  void create(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& callback);
  void join(const HttpRequestPtr& req,
            std::function<void(const HttpResponsePtr&)>&& callback,
            int sessionID);
};

} // namespace kamisado
