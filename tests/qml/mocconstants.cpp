/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QString>

#include "constants.h"

namespace {
bool s_productionMode = true;
}  // namespace

bool Constants::inProduction() { return s_productionMode; }

const QString& Constants::getStagingServerAddress() {
  static QString stagingServerAddress = Constants::API_STAGING_URL;
  return stagingServerAddress;
}

void Constants::setVersionOverride(const QString& versionOverride) {
  Q_UNUSED(versionOverride);
}

QString Constants::apiBaseUrl() { return Constants::API_STAGING_URL; }

QString Constants::apiUrl(ApiEndpoint) { return "something here"; }

void Constants::setStaging() { s_productionMode = false; }
void Constants::setStaging() { s_productionMode = false; }

QString Constants::versionString() {
  return QStringLiteral("QMLTest_AppVersion");
}

QString Constants::buildNumber() { return QStringLiteral("QMLTest_BuildID"); }

QString Constants::envOrDefault(const QString& name,
                                const QString& defaultValue) {
  Q_UNUSED(name);
  return defaultValue;
}
