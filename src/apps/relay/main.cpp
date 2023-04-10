/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QApplication>
#include <QIcon>

#include "appconstants.h"
#include "glean/mzglean.h"
#include "leakdetector.h"
#include "localizer.h"
#include "loghandler.h"
#include "relayapp.h"
#include "settingsholder.h"

Q_DECL_EXPORT int main(int argc, char* argv[]) {
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif

  SettingsHolder settingsHolder;

  if (settingsHolder.stagingServer()) {
    AppConstants::setStaging();
    LogHandler::enableStderr();
  }

  MZGlean::registerLogHandler(LogHandler::rustMessageHandler);
  qInstallMessageHandler(LogHandler::messageQTHandler);

  // Ensure that external styling hints are disabled.
  qunsetenv("QT_STYLE_OVERRIDE");

#ifdef MZ_ANDROID
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

  QApplication app(argc, argv);

  QCoreApplication::setApplicationName("Relay App");
  QCoreApplication::setApplicationVersion(Constants::versionString());

  Localizer localizer;

  QIcon icon(AppConstants::LOGO_URL);
  app.setWindowIcon(icon);

  RelayApp relayApp;
  if (!relayApp.initialize()) {
    return 1;
  }

  return app.exec();
}
