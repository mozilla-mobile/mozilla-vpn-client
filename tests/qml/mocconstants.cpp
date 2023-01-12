/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QString>

#include "appconstants.h"

bool Constants::inProduction() { return false; }

const QString& AppConstants::getStagingServerAddress() {
  static QString stagingServerAddress = AppConstants::API_STAGING_URL;
  return stagingServerAddress;
}

QString AppConstants::apiBaseUrl() { return AppConstants::API_STAGING_URL; }

QString AppConstants::apiUrl(ApiEndpoint) { return "something here"; }

void Constants::setStaging() {}
void AppConstants::setStaging() {}

QString Constants::versionString() {
  return QStringLiteral("QMLTest_AppVersion");
}

QString Constants::buildNumber() { return QStringLiteral("QMLTest_BuildID"); }

QString Constants::envOrDefault(const QString& name,
                                const QString& defaultValue) {
  Q_UNUSED(name);
  return defaultValue;
}
