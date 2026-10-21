#include <cstdlib>

#include <QApplication>
#include <QString>
#include <QUrl>

#include "agentim/base/application_info.h"

#include "agentim/ui/main_window.h"

#include "agentim/web/cef_browser_widget.h"
#include "agentim/web/cef_runtime.h"

namespace {

constexpr char kPrimaryWebApplicationId[] = "workbench";

}  // namespace

int main(int argc, char* argv[]) {
    const int secondary_process_exit_code =
        agentim::web::CefRuntime::executeSecondaryProcess(argc, argv);
    if (secondary_process_exit_code >= 0) {
        return secondary_process_exit_code;
    }

    QApplication application(argc, argv);

    const auto application_name = agentim::base::application_name();
    application.setApplicationName(QString::fromUtf8(
        application_name.data(), static_cast<qsizetype>(application_name.size())));
    application.setOrganizationName(QStringLiteral("AgentIM"));

    agentim::web::CefRuntime cef_runtime;
    if (!cef_runtime.initialize(argc, argv)) {
        return EXIT_FAILURE;
    }

    int exit_code = EXIT_SUCCESS;
    {
        const QUrl web_application_url = cef_runtime.applicationUrl(
            QString::fromLatin1(kPrimaryWebApplicationId));
        if (web_application_url.isEmpty() || !web_application_url.isValid()) {
            cef_runtime.shutdown();
            return EXIT_FAILURE;
        }

        auto* web_workspace =
            new agentim::web::CefBrowserWidget(web_application_url);
        agentim::ui::MainWindow main_window(web_workspace);
        main_window.show();

        exit_code = application.exec();
    }

    cef_runtime.shutdown();

    return exit_code;
}
