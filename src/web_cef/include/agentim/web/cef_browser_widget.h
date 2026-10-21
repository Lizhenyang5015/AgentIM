#pragma once

#include <memory>

#include <QUrl>
#include <QWidget>

class QFocusEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;

namespace agentim::web {

class CefBrowserWidget final : public QWidget {
public:
    explicit CefBrowserWidget(const QUrl& initial_url, QWidget* parent = nullptr);
    ~CefBrowserWidget() override;

    CefBrowserWidget(const CefBrowserWidget&) = delete;
    CefBrowserWidget& operator=(const CefBrowserWidget&) = delete;

protected:
    void focusInEvent(QFocusEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace agentim::web
