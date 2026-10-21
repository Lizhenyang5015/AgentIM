#include "include/cef_app.h"
#include "include/wrapper/cef_library_loader.h"

#include "agentim/web/cef_custom_scheme.h"

namespace {

class AgentCefHelperApp final : public CefApp {
public:
    void OnRegisterCustomSchemes(
        CefRawPtr<CefSchemeRegistrar> registrar) override {
        agentim::web::registerWebApplicationScheme(registrar);
    }

private:
    IMPLEMENT_REFCOUNTING(AgentCefHelperApp);
};

}  // namespace

int main(int argc, char* argv[]) {
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper()) {
        return 1;
    }

    const CefMainArgs main_arguments(argc, argv);
    CefRefPtr<AgentCefHelperApp> application = new AgentCefHelperApp;
    return CefExecuteProcess(main_arguments, application, nullptr);
}
