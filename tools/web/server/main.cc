#include "utils/Utils.h"
#include <drogon/drogon.h>
#include <filesystem>

namespace {

void addCorsHeaders(const drogon::HttpRequestPtr& req,
                    const drogon::HttpResponsePtr& resp) {
  const auto origin = req->getHeader("origin");
  if (!origin.empty() && kamisado::allowedOrigins.contains(origin)) {
    resp->addHeader("Access-Control-Allow-Origin", origin);
    resp->addHeader("Access-Control-Allow-Credentials", "true");
    resp->addHeader("Vary", "Origin");
  }
}

} // namespace

auto main(int /*argc*/, char** argv) -> int {

  drogon::app().registerPreRoutingAdvice(
      [](const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& cb,
         drogon::AdviceChainCallback&& pass) {
        if (req->method() != drogon::Options) {
          pass();
          return;
        }

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k204NoContent);

        addCorsHeaders(req, resp);

        resp->addHeader("Access-Control-Allow-Methods", "GET,POST");
        resp->addHeader("Access-Control-Allow-Headers",
                        "Content-Type, Authorization");
        resp->addHeader("Access-Control-Max-Age", "600");

        cb(resp);
      });

  drogon::app().registerPostHandlingAdvice(
      [](const drogon::HttpRequestPtr& req,
         const drogon::HttpResponsePtr& resp) {
        addCorsHeaders(req, resp);
      });

  drogon::app().addListener("0.0.0.0", 8081);
  std::filesystem::path binPath{ argv[0] };
  drogon::app().loadConfigFile(binPath.parent_path() / "config.yaml");
  drogon::app().run();
  return 0;
}
