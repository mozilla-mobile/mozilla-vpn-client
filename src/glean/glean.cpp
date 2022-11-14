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
#if defined(UNIT_TEST)
    // Clean the directory so test state doesn't leak
    gleanDirectory.removeRecursively();
#endif

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

#if defined(UNIT_TEST)
    glean_test_reset_glean(uploadEnabled, dataPath.toLocal8Bit());
#else
    glean_initialize(uploadEnabled, dataPath.toLocal8Bit(), appChannel);
#endif
  }
}

// static
void Glean::setUploadEnabled(bool isTelemetryEnabled) {
  logger.debug() << "Changing Glean upload status to" << isTelemetryEnabled;

  glean_set_upload_enabled(isTelemetryEnabled);
}
