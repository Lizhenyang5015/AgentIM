#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFocusEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QThread>
#include <QTimer>

#if defined(Q_OS_WIN)
#include <Windows.h>
#endif

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
#include "include/internal/cef_types_wrappers.h"

#include "agentim/web/cef_browser_widget.h"

namespace agentim::web {

#if defined(Q_OS_MACOS)
void resizeMacBrowserView(void* view_handle, int width, int height);
#endif

namespace {

constexpr int kBrowserShutdownTimeoutMs = 3000;

class CefBrowserClient final : public CefClient,
                               public CefLifeSpanHandler,
                               public CefLoadHandler {
public:
    explicit CefBrowserClient(std::function<void()> browser_created_callback)
        : browser_created_callback_(std::move(browser_created_callback)) {}

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }

    CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return this;
    }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        browser_ = browser;
        browser_created_callback_();
        qInfo() << "CEF browser created" << browser->GetIdentifier();
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
        if (browser_ && browser_->IsSame(browser)) {
            browser_ = nullptr;
            qInfo() << "CEF browser closed";
        }
    }

    void OnLoadEnd(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int http_status_code) override {
        Q_UNUSED(browser);
        if (frame->IsMain()) {
            qInfo() << "CEF page loaded" << http_status_code
                    << QString::fromStdString(frame->GetURL());
        }
    }

    void OnLoadError(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        ErrorCode error_code,
        const CefString& error_text,
        const CefString& failed_url) override {
        Q_UNUSED(browser);
        if (frame->IsMain()) {
            qWarning() << "CEF page load failed" << error_code
                       << QString::fromStdString(error_text)
                       << QString::fromStdString(failed_url);
        }
    }

    [[nodiscard]] CefRefPtr<CefBrowser> browser() const {
        return browser_;
    }

private:
    std::function<void()> browser_created_callback_;
    CefRefPtr<CefBrowser> browser_;

    IMPLEMENT_REFCOUNTING(CefBrowserClient);
};

void resizeNativeBrowser(CefRefPtr<CefBrowser> browser, int width, int height) {
    if (!browser) {
        return;
    }

#if defined(Q_OS_WIN)
    const auto window = browser->GetHost()->GetWindowHandle();
    SetWindowPos(window, nullptr, 0, 0, width, height, SWP_NOZORDER);
#elif defined(Q_OS_MACOS)
    resizeMacBrowserView(
        browser->GetHost()->GetWindowHandle(), width, height);
    browser->GetHost()->NotifyMoveOrResizeStarted();
#else
    Q_UNUSED(width);
    Q_UNUSED(height);
    browser->GetHost()->NotifyMoveOrResizeStarted();
#endif
}

}  // namespace

class CefBrowserWidget::Impl final {
public:
    Impl(CefBrowserWidget* owner, const QUrl& initial_url)
        : owner_(owner),
          initial_url_(initial_url.toString(QUrl::FullyEncoded).toStdString()),
          client_(new CefBrowserClient([this] {
              resizeBrowser();
          })) {}

    void createBrowser() {
        if (creation_requested_) {
            return;
        }

        creation_requested_ = true;
        owner_->setAttribute(Qt::WA_NativeWindow, true);

        const auto native_window = reinterpret_cast<CefWindowHandle>(
            static_cast<std::uintptr_t>(owner_->winId()));

        CefWindowInfo window_info;
        window_info.SetAsChild(
            native_window,
            CefRect(0, 0, owner_->width(), owner_->height()));
        window_info.runtime_style = CEF_RUNTIME_STYLE_ALLOY;

        CefBrowserSettings browser_settings;
        browser_settings.background_color = CefColorSetARGB(255, 255, 255, 255);

        qInfo() << "Creating CEF browser for"
                << QString::fromStdString(initial_url_);

        if (!CefBrowserHost::CreateBrowser(
                window_info,
                client_,
                initial_url_,
                browser_settings,
                nullptr,
                nullptr)) {
            creation_requested_ = false;
        }
    }

    void resizeBrowser() const {
        resizeNativeBrowser(client_->browser(), owner_->width(), owner_->height());
    }

    void notifyMoveOrResize() const {
        if (const auto browser = client_->browser()) {
            browser->GetHost()->NotifyMoveOrResizeStarted();
        }
    }

    void focusBrowser() const {
        if (const auto browser = client_->browser()) {
            browser->GetHost()->SetFocus(true);
        }
    }

    void closeBrowser() {
        auto browser = client_->browser();
        if (!browser) {
            return;
        }

        browser->GetHost()->CloseBrowser(true);

        QElapsedTimer timer;
        timer.start();
        while (client_->browser() && timer.elapsed() < kBrowserShutdownTimeoutMs) {
            CefDoMessageLoopWork();
            QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
            QThread::msleep(1);
        }
    }

private:
    CefBrowserWidget* owner_ = nullptr;
    std::string initial_url_;
    CefRefPtr<CefBrowserClient> client_;
    bool creation_requested_ = false;
};

CefBrowserWidget::CefBrowserWidget(const QUrl& initial_url, QWidget* parent)
    : QWidget(parent),
      impl_(std::make_unique<Impl>(this, initial_url)) {
    Q_ASSERT(initial_url.isValid());
    setAttribute(Qt::WA_NativeWindow, true);
    setFocusPolicy(Qt::StrongFocus);
}

CefBrowserWidget::~CefBrowserWidget() {
    impl_->closeBrowser();
}

void CefBrowserWidget::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    impl_->focusBrowser();
}

void CefBrowserWidget::moveEvent(QMoveEvent* event) {
    QWidget::moveEvent(event);
    impl_->notifyMoveOrResize();
}

void CefBrowserWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    impl_->resizeBrowser();
}

void CefBrowserWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    QTimer::singleShot(0, this, [this] {
        impl_->createBrowser();
    });
}

}  // namespace agentim::web
