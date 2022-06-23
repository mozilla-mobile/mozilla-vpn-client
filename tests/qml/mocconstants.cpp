/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "constants.h"

#include <QString>
#include <QtGlobal>

bool Constants::inProduction() { return false; }

const QString& Constants::getStagingServerAddress() {
  static QString stagingServerAddress = Constants::API_STAGING_URL;
  return stagingServerAddress;
}

void Constants::setStaging() {}

QString Constants::versionString() {
  return QStringLiteral("QMLTest_AppVersion");
}

QString Constants::buildNumber() { return QStringLiteral("QMLTest_BuildID"); }

QString Constants::envOrDefault(const QString& name,
                                const QString& defaultValue) {
  Q_UNUSED(name);
  return defaultValue;
}
