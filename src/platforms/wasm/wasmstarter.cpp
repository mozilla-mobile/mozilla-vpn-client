/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmstarter.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>

namespace {
Logger logger(LOG_MAIN, "WasmStarter");
}

int WasmStarter::s_authLoadingTimeoutMSec = 2000;

// static
void WasmStarter::initialize() {
  logger.debug() << "Configure wasm starter";

  QProcessEnvironment qpe = QProcessEnvironment::systemEnvironment();
  if (!qpe.contains("WASMCONFIG")) {
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(qpe.value("WASMCONFIG").toUtf8());
  QJsonObject obj = doc.object();

  if (obj["skipAuthentication"].toBool()) {
    logger.debug() << "WasmConfig - skip authentication";

    SettingsHolder* settingsHolder = SettingsHolder::instance();
    Q_ASSERT(settingsHolder);

    settingsHolder->setPostAuthenticationShown(true);
    settingsHolder->setTelemetryPolicyShown(true);

    s_authLoadingTimeoutMSec = 0;

    MozillaVPN::instance()->getStarted();
  }
}
