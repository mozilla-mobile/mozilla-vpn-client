/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// TODO: Stop using the relative path once Glean.js is fully removed from the
// codebase. Until then we need the relative path, otherwise XCode is confused
// about what is being imported.
#include "./glean.h"

#include "glean/generated/metrics.h"
#include "glean/generated/pings.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#if not(defined(MVPN_WASM) || defined(BUILD_QMAKE))
#  include "vpnglean.h"
#endif
#if defined(MVPN_IOS) && not(defined(BUILD_QMAKE))
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
  MVPN_COUNT_CTOR(VPNGlean);
}

VPNGlean::~VPNGlean() { MVPN_COUNT_DTOR(VPNGlean); }

// static
void VPNGlean::initialize() {
  logger.debug() << "Initializing VPNGlean";

  registerQMLSingletons();

  if (Feature::get(Feature::Feature_gleanRust)->isSupported()) {
    QDir gleanDirectory(rootAppFolder());
#if defined(UNIT_TEST)
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

    s_instance = new VPNGlean(qApp);
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
#elif defined(MVPN_IOS) && not(defined(BUILD_QMAKE))
    new IOSGleanBridge(uploadEnabled, appChannel);
#elif not(defined(MVPN_WASM) || defined(BUILD_QMAKE))
    glean_initialize(uploadEnabled, dataPath.toLocal8Bit(), appChannel);
#endif
  }
}

// static
void VPNGlean::setUploadEnabled(bool isTelemetryEnabled) {
  logger.debug() << "Changing VPNGlean upload status to" << isTelemetryEnabled;

#if not(defined(MVPN_WASM) || defined(BUILD_QMAKE))
  glean_set_upload_enabled(isTelemetryEnabled);
#endif
}

// static
void VPNGlean::registerQMLSingletons() {
  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "GleanPings",
      [](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = __DONOTUSE__GleanPings::instance();
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });

  qmlRegisterSingletonType<MozillaVPN>(
      "Mozilla.VPN", 1, 0, "Glean", [](QQmlEngine*, QJSEngine*) -> QObject* {
        QObject* obj = __DONOTUSE__GleanMetrics::instance();
        QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
        return obj;
      });
}
