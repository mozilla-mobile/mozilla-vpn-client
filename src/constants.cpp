/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "constants.h"
#include "settingsholder.h"
#include "version.h"

#include <QProcessEnvironment>
#include <QString>
#include <QtGlobal>
#include <QUrl>

namespace {
bool s_inProduction = true;
QString s_stagingServerAddress = "";
}  // namespace

bool Constants::inProduction() { return s_inProduction; }

const QString& Constants::getStagingServerAddress() {
  return s_stagingServerAddress;
}

void Constants::setStaging() {
  s_inProduction = false;
  s_stagingServerAddress = SettingsHolder::instance()->stagingServerAddress();
  Q_ASSERT(!s_stagingServerAddress.isEmpty());
}

QString Constants::versionString() { return QStringLiteral(APP_VERSION); }

QString Constants::buildNumber() { return QStringLiteral(BUILD_ID); }

QString Constants::envOrDefault(const QString& name,
                                const QString& defaultValue) {
  QString env;

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains(name)) {
    env = pe.value(name);
  }

  if (env.isEmpty()) {
    return defaultValue;
  }

  if (!QUrl(env).isValid()) {
    return defaultValue;
  }

  return env;
}
