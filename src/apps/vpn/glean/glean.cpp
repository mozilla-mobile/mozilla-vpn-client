/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// TODO: Stop using the relative path once Glean.js is fully removed from the
// codebase. Until then we need the relative path, otherwise XCode is confused
// about what is being imported.
#include "./glean.h"

#include "constants.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
#  include "vpnglean.h"
#endif
#if defined(MZ_IOS) && not(defined(BUILD_QMAKE))
#  include "platforms/ios/iosgleanbridge.h"
#endif

#include <QCoreApplication>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>

namespace {
Logger logger("Glean");
VPNGlean* s_instance = nullptr;

QString rootAppFolder() {
#if defined(UNIT_TEST)
  return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}
}  // namespace

VPNGlean::VPNGlean(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(VPNGlean);
}

VPNGlean::~VPNGlean() { MZ_COUNT_DTOR(VPNGlean); }

// static
void VPNGlean::initialize() {
  logger.debug() << "Initializing VPNGlean";

  if (Feature::get(Feature::Feature_gleanRust)->isSupported()) {
    if (!s_instance) {
      s_instance = new VPNGlean(qApp);

      connect(SettingsHolder::instance(), &SettingsHolder::gleanEnabledChanged,
              s_instance, []() {
                s_instance->setUploadEnabled(
                    SettingsHolder::instance()->gleanEnabled());
              });
    }

    QDir gleanDirectory(rootAppFolder());
#if defined(UNIT_TEST)
    logger.debug() << "Cleaning Glean directory for testing";
    // Clean the directory so test state doesn't leak
    // See https://bugzilla.mozilla.org/show_bug.cgi?id=1800901
    gleanDirectory.removeRecursively();
#endif

    if (!gleanDirectory.exists(GLEAN_DATA_DIRECTORY) &&
        !gleanDirectory.mkpath(GLEAN_DATA_DIRECTORY)) {
      logger.error()
          << "Unable to create the VPNGlean data directory. Terminating."
          << rootAppFolder();
      return;
    }

    if (!gleanDirectory.cd(GLEAN_DATA_DIRECTORY)) {
      logger.error()
          << "Unable to open the VPNGlean data directory. Terminating.";
      return;
    }

    auto uploadEnabled = SettingsHolder::instance()->gleanEnabled();
    auto appChannel = Constants::inProduction() ? "production" : "staging";
    auto dataPath = gleanDirectory.absolutePath();

#if defined(UNIT_TEST)
    glean_test_reset_glean(uploadEnabled, dataPath.toLocal8Bit());
#elif defined(MZ_IOS) && not(defined(BUILD_QMAKE))
    new IOSGleanBridge(uploadEnabled, appChannel);
#elif not(defined(MZ_WASM) || defined(BUILD_QMAKE))
    glean_initialize(uploadEnabled, dataPath.toLocal8Bit(), appChannel);
#endif
  }
}

// static
void VPNGlean::setUploadEnabled(bool isTelemetryEnabled) {
  logger.debug() << "Changing VPNGlean upload status to" << isTelemetryEnabled;

#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  glean_set_upload_enabled(isTelemetryEnabled);
#endif
}

// static
void VPNGlean::shutdown() {
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  glean_shutdown();
#endif
}
