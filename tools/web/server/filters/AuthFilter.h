/**
 *
 *  kamisado_AuthFilter.h
 *
 */

#pragma once

#include <drogon/HttpFilter.h>
using namespace drogon;
namespace kamisado
{

class AuthFilter : public HttpFilter<AuthFilter>
{
  public:
    AuthFilter() = default;
    void doFilter(const HttpRequestPtr& req, FilterCallback&& fcb,
                  FilterChainCallback&& fccb) override;
};

}
