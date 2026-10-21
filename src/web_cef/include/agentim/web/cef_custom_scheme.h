#pragma once

#include "include/cef_scheme.h"

namespace agentim::web {

inline constexpr char kWebApplicationScheme[] = "agentim";

inline bool registerWebApplicationScheme(
    CefRawPtr<CefSchemeRegistrar> registrar) {
    constexpr int kSchemeOptions =
        CEF_SCHEME_OPTION_STANDARD |
        CEF_SCHEME_OPTION_SECURE |
        CEF_SCHEME_OPTION_CORS_ENABLED |
        CEF_SCHEME_OPTION_FETCH_ENABLED;
    return registrar != nullptr && registrar->AddCustomScheme(
        kWebApplicationScheme, kSchemeOptions);
}

}  // namespace agentim::web
