/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "constants.h"

#include <QProcessEnvironment>
#include <QString>
#include <QtGlobal>

#include "settingsholder.h"
#include "version.h"

namespace {
bool s_inProduction = true;
}  // namespace

bool Constants::inProduction() { return s_inProduction; }

void Constants::setStaging() { s_inProduction = false; }

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

  return env;
}
