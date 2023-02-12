/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mzglean.h"

#include "constants.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
#  include "vpnglean.h"
#endif
#if defined(MZ_ANDROID) && not(defined(BUILD_QMAKE))
#  include "platforms/android/androidutils.h"
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
MZGlean* s_instance = nullptr;

QString rootAppFolder() {
#if defined(UNIT_TEST)
  return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}
}  // namespace

MZGlean::MZGlean(QObject* parent) : QObject(parent) { MZ_COUNT_CTOR(MZGlean); }

MZGlean::~MZGlean() { MZ_COUNT_DTOR(MZGlean); }

// static
void MZGlean::registerLogHandler(void (*messageHandler)(int32_t, char*)) {
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  glean_register_log_handler(messageHandler);
#endif
}

// static
void MZGlean::initialize() {
  logger.debug() << "Initializing MZGlean";

  if (Feature::get(Feature::Feature_gleanRust)->isSupported()) {
    if (!s_instance) {
      s_instance = new MZGlean(qApp);

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
          << "Unable to create the MZGlean data directory. Terminating."
          << rootAppFolder();
      return;
    }

    if (!gleanDirectory.cd(GLEAN_DATA_DIRECTORY)) {
      logger.error()
          << "Unable to open the MZGlean data directory. Terminating.";
      return;
    }

#if defined(UNIT_TEST)
    glean_test_reset_glean(SettingsHolder::instance()->gleanEnabled(),
                           gleanDirectory.absolutePath().toLocal8Bit());
#elif defined(MZ_IOS) && not(defined(BUILD_QMAKE))
    new IOSGleanBridge(SettingsHolder::instance()->gleanEnabled(),
                       Constants::inProduction() ? "production" : "staging");
#elif defined(MZ_ANDROID) && not(defined(BUILD_QMAKE))
    AndroidUtils::initializeGlean(
        SettingsHolder::instance()->gleanEnabled(),
        Constants::inProduction() ? "production" : "staging");
#elif not(defined(MZ_WASM) || defined(BUILD_QMAKE))
    glean_initialize(SettingsHolder::instance()->gleanEnabled(),
                     gleanDirectory.absolutePath().toLocal8Bit(),
                     Constants::inProduction() ? "production" : "staging");
#endif
  }
}

// static
void MZGlean::setUploadEnabled(bool isTelemetryEnabled) {
  logger.debug() << "Changing MZGlean upload status to" << isTelemetryEnabled;

#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  glean_set_upload_enabled(isTelemetryEnabled);
#endif
}

// static
void MZGlean::shutdown() {
#if not(defined(MZ_WASM) || defined(BUILD_QMAKE))
  glean_shutdown();
#endif
}
