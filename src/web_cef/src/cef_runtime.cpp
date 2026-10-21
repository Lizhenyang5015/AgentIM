#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QMetaObject>
#include <QStandardPaths>
#include <QString>
#include <QThread>
#include <QTimer>

#if defined(Q_OS_WIN)
#include <Windows.h>
#endif

#include "include/cef_app.h"
#include "include/internal/cef_types_wrappers.h"
#if defined(Q_OS_MACOS)
#include "include/wrapper/cef_library_loader.h"
#endif

#include "agentim/web/cef_custom_scheme.h"
#include "agentim/web/cef_runtime.h"
#include "packaged_web_app_registry.h"

namespace agentim::web {

namespace {

constexpr int kMessagePumpFallbackIntervalMs = 10;

class CefMessagePump final : public QObject {
public:
    explicit CefMessagePump(QObject* parent = nullptr)
        : QObject(parent) {
        timer_.setSingleShot(true);
        QObject::connect(&timer_, &QTimer::timeout, this, [this] {
            runWork();
        });
    }

    void schedule(std::int64_t delay_ms) {
        if (QThread::currentThread() != thread()) {
            QMetaObject::invokeMethod(
                this,
                [this, delay_ms] {
                    schedule(delay_ms);
                },
                Qt::QueuedConnection);
            return;
        }

        if (work_running_) {
            work_pending_ = true;
            return;
        }

        const auto bounded_delay = std::clamp<std::int64_t>(
            delay_ms,
            0,
            std::numeric_limits<int>::max());
        const int delay = static_cast<int>(bounded_delay);
        if (!timer_.isActive() || timer_.remainingTime() > delay) {
            timer_.start(delay);
        }
    }

    void stop() {
        timer_.stop();
    }

private:
    void runWork() {
        if (work_running_) {
            work_pending_ = true;
            return;
        }

        work_running_ = true;
        work_pending_ = false;
        CefDoMessageLoopWork();
        work_running_ = false;

        if (work_pending_) {
            schedule(0);
        } else {
            schedule(kMessagePumpFallbackIntervalMs);
        }
    }

    QTimer timer_;
    bool work_running_ = false;
    bool work_pending_ = false;
};

class AgentCefApp final : public CefApp,
                          public CefBrowserProcessHandler {
public:
    explicit AgentCefApp(CefMessagePump* message_pump)
        : message_pump_(message_pump) {}

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    void OnRegisterCustomSchemes(
        CefRawPtr<CefSchemeRegistrar> registrar) override {
        registerWebApplicationScheme(registrar);
    }

    void OnScheduleMessagePumpWork(std::int64_t delay_ms) override {
        message_pump_->schedule(delay_ms);
    }

private:
    CefMessagePump* message_pump_ = nullptr;

    IMPLEMENT_REFCOUNTING(AgentCefApp);
};

class AgentCefSecondaryProcessApp final : public CefApp {
public:
    void OnRegisterCustomSchemes(
        CefRawPtr<CefSchemeRegistrar> registrar) override {
        registerWebApplicationScheme(registrar);
    }

private:
    IMPLEMENT_REFCOUNTING(AgentCefSecondaryProcessApp);
};

QString applicationDataDirectory() {
    const QString data_root =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir directory(data_root);
    directory.mkpath(QStringLiteral("cef/cache"));
    directory.mkpath(QStringLiteral("cef/log"));
    return directory.absolutePath();
}

}  // namespace

class CefRuntime::Impl final {
public:
    bool initialize(int argc, char* argv[]) {
        if (initialized_) {
            return true;
        }

#if defined(Q_OS_MACOS)
        library_loader_ = std::make_unique<CefScopedLibraryLoader>();
        if (!library_loader_->LoadInMain()) {
            library_loader_.reset();
            return false;
        }
#endif

        message_pump_ = std::make_unique<CefMessagePump>();
        application_ = new AgentCefApp(message_pump_.get());

        const QString data_directory = applicationDataDirectory();
        const QString cache_directory =
            QDir(data_directory).absoluteFilePath(QStringLiteral("cef/cache"));
        const QString log_file =
            QDir(data_directory).absoluteFilePath(QStringLiteral("cef/log/cef.log"));

        CefSettings settings;
        settings.external_message_pump = true;
        // Development bundles are not code-signed with the CEF sandbox
        // entitlements yet. Release packaging must enable the sandbox after it
        // owns signing and hardened-runtime configuration.
        settings.no_sandbox = true;

        CefString(&settings.root_cache_path) = cache_directory.toStdString();
        CefString(&settings.cache_path) = cache_directory.toStdString();
        CefString(&settings.log_file) = log_file.toStdString();
        CefString(&settings.locale) = QLocale::system().bcp47Name().toStdString();

#if defined(Q_OS_MACOS)
        const QString helper_path = QDir(QCoreApplication::applicationDirPath())
                                        .absoluteFilePath(QStringLiteral(
                                            "../Frameworks/AgentIM Helper.app/Contents/"
                                            "MacOS/AgentIM Helper"));
        CefString(&settings.browser_subprocess_path) = helper_path.toStdString();
#endif

#if defined(Q_OS_WIN)
        const CefMainArgs main_arguments(GetModuleHandle(nullptr));
#else
        const CefMainArgs main_arguments(argc, argv);
#endif

        if (!CefInitialize(main_arguments, settings, application_, nullptr)) {
            application_ = nullptr;
            message_pump_.reset();
#if defined(Q_OS_MACOS)
            library_loader_.reset();
#endif
            return false;
        }

        if (!registerPackagedWebApplications()) {
            CefShutdown();
            application_ = nullptr;
            message_pump_.reset();
#if defined(Q_OS_MACOS)
            library_loader_.reset();
#endif
            return false;
        }

        message_pump_->schedule(0);
        initialized_ = true;
        return true;
    }

    void shutdown() {
        if (!initialized_) {
            return;
        }

        unregisterPackagedWebApplications();
        message_pump_->stop();
        CefShutdown();
        application_ = nullptr;
        message_pump_.reset();
#if defined(Q_OS_MACOS)
        library_loader_.reset();
#endif
        initialized_ = false;
    }

    [[nodiscard]] bool isInitialized() const {
        return initialized_;
    }

private:
    bool initialized_ = false;
    std::unique_ptr<CefMessagePump> message_pump_;
    CefRefPtr<AgentCefApp> application_;
#if defined(Q_OS_MACOS)
    std::unique_ptr<CefScopedLibraryLoader> library_loader_;
#endif
};

CefRuntime::CefRuntime()
    : impl_(std::make_unique<Impl>()) {}

CefRuntime::~CefRuntime() {
    shutdown();
}

int CefRuntime::executeSecondaryProcess(int argc, char* argv[]) {
#if defined(Q_OS_WIN)
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    const CefMainArgs main_arguments(GetModuleHandle(nullptr));
    CefRefPtr<AgentCefSecondaryProcessApp> application =
        new AgentCefSecondaryProcessApp;
    return CefExecuteProcess(main_arguments, application, nullptr);
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    return -1;
#endif
}

bool CefRuntime::initialize(int argc, char* argv[]) {
    return impl_->initialize(argc, argv);
}

void CefRuntime::shutdown() {
    impl_->shutdown();
}

bool CefRuntime::isInitialized() const {
    return impl_->isInitialized();
}

QUrl CefRuntime::applicationUrl(const QString& application_id) const {
    return isInitialized() ? packagedWebApplicationUrl(application_id) : QUrl{};
}

}  // namespace agentim::web
