/**
 *
 *  kamisado_AuthFilter.cc
 *
 */

#include "AuthFilter.h"
#include "drogon/HttpAppFramework.h"
#include "drogon/HttpTypes.h"
#include "plugins/SessionManagerPlugin.h"
#include "utils/Utils.h"

using namespace drogon;
using namespace kamisado;

void AuthFilter::doFilter(const HttpRequestPtr& req, FilterCallback&& fcb,
                          FilterChainCallback&& fccb) {
  auto err = [&] {
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    fcb(res);
  };

  auto token = parseToken(req);
  if (token.empty()) {
    err();
    return;
  }
  auto authRes = app().getPlugin<SessionManagerPlugin>()->auth(token);

  if (authRes) {
    // Passed
    fccb();
    return;
  }

  err();
}
