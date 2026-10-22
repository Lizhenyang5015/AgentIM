#include <optional>
#include <utility>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QUrl>

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_request.h"
#include "include/cef_scheme.h"
#include "include/cef_stream.h"
#include "include/wrapper/cef_stream_resource_handler.h"

#include "agentim/web/cef_custom_scheme.h"
#include "packaged_web_app_registry.h"

namespace agentim::web {

namespace {

constexpr int kSupportedManifestSchemaVersion = 1;
constexpr char kManifestFileName[] = "app.manifest.json";
constexpr char kCefRuntimeName[] = "cef";

struct PackagedWebApplication final {
    QString application_id;
    QString entry_point;
    QDir root_directory;
    QString canonical_root;
};

QSet<QString> registered_application_ids;

QString stagedWebApplicationsRoot() {
#if defined(Q_OS_MACOS)
    return QDir(QCoreApplication::applicationDirPath())
        .absoluteFilePath(QStringLiteral("../Resources/web"));
#else
    return QDir(QCoreApplication::applicationDirPath())
        .absoluteFilePath(QStringLiteral("resources/web"));
#endif
}

bool isValidApplicationId(const QString& application_id) {
    static const QRegularExpression pattern(
        QStringLiteral("^[a-z][a-z0-9-]*$"));
    return pattern.match(application_id).hasMatch();
}

bool isSafeRelativePath(const QString& path) {
    return !path.isEmpty() && path != QStringLiteral("..") &&
           !path.startsWith(QStringLiteral("../")) &&
           !QDir::isAbsolutePath(path);
}

bool isFileInsideApplication(
    const PackagedWebApplication& application,
    const QFileInfo& file) {
    const QString canonical_file = file.canonicalFilePath();
    // Qt canonical paths use '/' on every platform, including Windows.
    const QString canonical_root_prefix =
        application.canonical_root + QStringLiteral("/");
    return !application.canonical_root.isEmpty() &&
           !canonical_file.isEmpty() &&
           canonical_file.startsWith(canonical_root_prefix) && file.isFile();
}

std::optional<PackagedWebApplication> loadApplicationManifest(
    const QDir& application_directory) {
    const QString manifest_path =
        application_directory.filePath(QString::fromLatin1(kManifestFileName));
    QFile manifest_file(manifest_path);
    if (!manifest_file.open(QIODevice::ReadOnly)) {
        qWarning() << "Packaged Web application manifest could not be opened"
                   << manifest_path;
        return std::nullopt;
    }

    QJsonParseError parse_error;
    const QJsonDocument manifest =
        QJsonDocument::fromJson(manifest_file.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !manifest.isObject()) {
        qWarning() << "Packaged Web application manifest is invalid"
                   << manifest_path << parse_error.errorString();
        return std::nullopt;
    }

    const QJsonObject manifest_object = manifest.object();
    const int schema_version =
        manifest_object.value(QStringLiteral("schemaVersion")).toInt(-1);
    const QString application_id =
        manifest_object.value(QStringLiteral("appId")).toString();
    const QString runtime =
        manifest_object.value(QStringLiteral("runtime")).toString();
    const QString entry_point = QDir::cleanPath(
        manifest_object.value(QStringLiteral("entryPoint")).toString());

    if (schema_version != kSupportedManifestSchemaVersion ||
        !isValidApplicationId(application_id) ||
        application_directory.dirName() != application_id ||
        runtime != QString::fromLatin1(kCefRuntimeName) ||
        !isSafeRelativePath(entry_point)) {
        qWarning() << "Packaged Web application manifest contract is invalid"
                   << manifest_path;
        return std::nullopt;
    }

    PackagedWebApplication application{
        application_id,
        entry_point,
        application_directory,
        application_directory.canonicalPath(),
    };
    const QFileInfo entry_file(application_directory.filePath(entry_point));
    if (!isFileInsideApplication(application, entry_file)) {
        qWarning() << "Packaged Web application entry point is missing"
                   << entry_file.absoluteFilePath();
        return std::nullopt;
    }

    return application;
}

std::optional<QHash<QString, PackagedWebApplication>>
loadPackagedWebApplications() {
    const QDir applications_root(stagedWebApplicationsRoot());
    if (!applications_root.exists()) {
        qWarning() << "Packaged Web applications root is missing"
                   << applications_root.absolutePath();
        return std::nullopt;
    }

    QHash<QString, PackagedWebApplication> applications;
    const QFileInfoList application_directories = applications_root.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name);
    for (const QFileInfo& directory_info : application_directories) {
        auto application =
            loadApplicationManifest(QDir(directory_info.absoluteFilePath()));
        if (!application || applications.contains(application->application_id)) {
            return std::nullopt;
        }

        const QString application_id = application->application_id;
        applications.insert(application_id, std::move(*application));
    }

    if (applications.isEmpty()) {
        qWarning() << "No packaged Web applications were discovered under"
                   << applications_root.absolutePath();
        return std::nullopt;
    }

    return applications;
}

class PackagedWebApplicationSchemeHandlerFactory final
    : public CefSchemeHandlerFactory {
public:
    explicit PackagedWebApplicationSchemeHandlerFactory(
        QHash<QString, PackagedWebApplication> applications)
        : applications_(std::move(applications)) {}

    CefRefPtr<CefResourceHandler> Create(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& scheme_name,
        CefRefPtr<CefRequest> request) override {
        Q_UNUSED(browser);
        Q_UNUSED(frame);

        if (scheme_name.ToString() != kWebApplicationScheme ||
            request->GetMethod().ToString() != "GET") {
            return nullptr;
        }

        const QUrl request_url(
            QString::fromStdString(request->GetURL().ToString()));
        const auto application = applications_.constFind(request_url.host());
        if (application == applications_.cend()) {
            return nullptr;
        }

        QString relative_path = request_url.path(QUrl::FullyDecoded);
        if (relative_path.startsWith('/')) {
            relative_path.remove(0, 1);
        }
        if (relative_path.isEmpty()) {
            relative_path = application->entry_point;
        }

        relative_path = QDir::cleanPath(relative_path);
        if (!isSafeRelativePath(relative_path)) {
            return nullptr;
        }

        QFileInfo requested_file(
            application->root_directory.filePath(relative_path));
        if (!isFileInsideApplication(*application, requested_file) &&
            request->GetResourceType() == RT_MAIN_FRAME) {
            requested_file = QFileInfo(
                application->root_directory.filePath(application->entry_point));
        }

        if (!isFileInsideApplication(*application, requested_file)) {
            qWarning() << "Packaged Web resource was not found" << request_url;
            return nullptr;
        }

        const QString canonical_file = requested_file.canonicalFilePath();
        CefRefPtr<CefStreamReader> stream =
            CefStreamReader::CreateForFile(canonical_file.toStdString());
        if (!stream) {
            qWarning() << "Packaged Web resource could not be opened"
                       << canonical_file;
            return nullptr;
        }

        QString mime_type =
            QMimeDatabase().mimeTypeForFile(requested_file).name();
        if (mime_type.isEmpty()) {
            mime_type = QStringLiteral("application/octet-stream");
        }

        CefResponse::HeaderMap response_headers;
        response_headers.emplace("Cache-Control", "no-store");
        response_headers.emplace("X-Content-Type-Options", "nosniff");

        qInfo() << "Loading packaged Web resource" << request_url;
        return new CefStreamResourceHandler(
            200,
            "OK",
            mime_type.toStdString(),
            std::move(response_headers),
            stream);
    }

private:
    const QHash<QString, PackagedWebApplication> applications_;

    IMPLEMENT_REFCOUNTING(PackagedWebApplicationSchemeHandlerFactory);
};

}  // namespace

bool registerPackagedWebApplications() {
    auto applications = loadPackagedWebApplications();
    if (!applications) {
        return false;
    }

    registered_application_ids.clear();
    for (auto application = applications->cbegin();
         application != applications->cend();
         ++application) {
        registered_application_ids.insert(application.key());
    }

    CefRefPtr<PackagedWebApplicationSchemeHandlerFactory> factory =
        new PackagedWebApplicationSchemeHandlerFactory(std::move(*applications));
    if (!CefRegisterSchemeHandlerFactory(
            kWebApplicationScheme,
            CefString{},
            factory)) {
        registered_application_ids.clear();
        qWarning() << "Packaged Web application registry failed";
        return false;
    }

    qInfo() << "Packaged Web applications registered"
            << registered_application_ids.values();
    return true;
}

void unregisterPackagedWebApplications() {
    CefRegisterSchemeHandlerFactory(
        kWebApplicationScheme,
        CefString{},
        nullptr);
    registered_application_ids.clear();
}

QUrl packagedWebApplicationUrl(const QString& application_id) {
    if (!registered_application_ids.contains(application_id)) {
        qWarning() << "Packaged Web application is not registered"
                   << application_id;
        return {};
    }

    QUrl url;
    url.setScheme(QString::fromLatin1(kWebApplicationScheme));
    url.setHost(application_id);
    url.setPath(QStringLiteral("/"));
    return url;
}

}  // namespace agentim::web
