#pragma once

#include <QString>

class QUrl;

namespace agentim::web {

bool registerPackagedWebApplications();
void unregisterPackagedWebApplications();
QUrl packagedWebApplicationUrl(const QString& application_id);

}  // namespace agentim::web
