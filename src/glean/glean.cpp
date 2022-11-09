/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "glean/glean.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "vpnglean.h"

#include <QDir>
#include <QStandardPaths>

namespace {
Logger logger(LOG_MAIN, "Glean");
Glean* s_instance = nullptr;

QString rootAppFolder() {
#if defined(UNIT_TEST)
  return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}
}  // namespace

Glean::Glean() { MVPN_COUNT_CTOR(Glean); }

Glean::~Glean() { MVPN_COUNT_DTOR(Glean); }
// static
void Glean::initialize() {
  logger.debug() << "Initializing Glean";

  if (Feature::get(Feature::Feature_gleanRust)->isSupported()) {
    QDir gleanDirectory(rootAppFolder());
    if (!gleanDirectory.exists(GLEAN_DATA_DIRECTORY) &&
        !gleanDirectory.mkpath(GLEAN_DATA_DIRECTORY)) {
      logger.error()
          << "Unable to create the Glean data directory. Terminating."
          << rootAppFolder();
      return;
    }

    if (!gleanDirectory.cd(GLEAN_DATA_DIRECTORY)) {
      logger.error() << "Unable to open the Glean data directory. Terminating.";
      return;
    }

    s_instance = new Glean();
    connect(SettingsHolder::instance(), &SettingsHolder::gleanEnabledChanged,
            s_instance, []() {
              s_instance->setUploadEnabled(
                  SettingsHolder::instance()->gleanEnabled());
            });

    SettingsHolder* settingsHolder = SettingsHolder::instance();
    MozillaVPN* vpn = MozillaVPN::instance();

    auto uploadEnabled = settingsHolder->gleanEnabled();
    auto appChannel = vpn->stagingMode() ? "staging" : "production";
    auto dataPath = gleanDirectory.absolutePath();

    logger.debug() << "Glean config -"
                   << "uploadEnabled:" << uploadEnabled
                   << "appChannel:" << appChannel << "dataPath:" << dataPath;

    glean_initialize(uploadEnabled, dataPath.toLocal8Bit(), appChannel);
  }
}

// static
void Glean::setUploadEnabled(bool isTelemetryEnabled) {
  logger.debug() << "Changing Glean upload status to" << isTelemetryEnabled;

  glean_set_upload_enabled(isTelemetryEnabled);
}
