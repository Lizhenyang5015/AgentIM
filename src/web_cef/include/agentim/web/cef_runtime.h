#pragma once

#include <memory>

#include <QString>
#include <QUrl>

namespace agentim::web {

class CefRuntime final {
public:
    CefRuntime();
    ~CefRuntime();

    CefRuntime(const CefRuntime&) = delete;
    CefRuntime& operator=(const CefRuntime&) = delete;

    static int executeSecondaryProcess(int argc, char* argv[]);

    bool initialize(int argc, char* argv[]);
    void shutdown();
    [[nodiscard]] bool isInitialized() const;
    [[nodiscard]] QUrl applicationUrl(const QString& application_id) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace agentim::web
