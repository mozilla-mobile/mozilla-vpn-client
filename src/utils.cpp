/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utils.h"

#include "app.h"
#include "constants.h"
#include "feature/feature.h"
#include "logger.h"
#include "loghandler.h"
#include "urlopener.h"

#ifdef MZ_ANDROID
#  include <android/log.h>

#  include "platforms/android/androidcommons.h"
#endif
#ifdef MZ_WINDOWS
#  include "platforms/windows/windowsutils.h"
#endif
#ifdef MZ_IOS
#  include "platforms/ios/ioscommons.h"
#  include "platforms/ios/ioslogger.h"
#endif

#include <QApplication>
#include <QClipboard>
#include <QUrl>

namespace {
Logger logger("Utils");
}

// static
Utils* Utils::instance() {
  static Utils s_instance;
  return &s_instance;
}

void Utils::exitForUnrecoverableError(const QString& reason) {
  Q_ASSERT(!reason.isEmpty());
  logger.error() << "Unrecoverable error detected: " << reason;
  App::instance()->quit();
}

// static
void Utils::storeInClipboard(const QString& text) {
  logger.debug() << "Store in clipboard";
  QApplication::clipboard()->setText(text);
}

// static
void Utils::crashTest() {
  logger.debug() << "Crashing Application";

#ifdef MZ_WINDOWS
  // Windows does not have "signals"
  WindowsUtils::forceCrash();
#else
  // On Linux/osx this generates a Sigabort, which is handled
  qFatal("Ready to crash!");
#endif
}

#ifdef MZ_ANDROID
// static
void Utils::launchPlayStore() {
  logger.debug() << "Launch Play Store";
  AndroidCommons::launchPlayStore();
}
#endif

bool Utils::viewLogs() {
  logger.debug() << "View logs";

  if (!Feature::get(Feature::Feature_shareLogs)->isSupported()) {
    logger.error() << "ViewLogs Called on unsupported OS or version!";
    return false;
  }

#if defined(MZ_ANDROID) || defined(MZ_IOS)
  bool ok = true;
  QBuffer* buffer = new QBuffer();
  buffer->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
  connect(buffer, &QIODevice::aboutToClose, buffer, &QObject::deleteLater);
#  if defined(MZ_ANDROID)
  connect(buffer, &QIODevice::aboutToClose, this,
          [&]() { ok = AndroidCommons::shareText(QString(buffer->data())); });
#  elif defined(MZ_IOS)
  connect(buffer, &QIODevice::aboutToClose, this,
          [&]() { IOSCommons::shareLogs(QString(buffer->data())); });
#  endif
  LogHandler::instance()->logSerialize(buffer);
  return ok;
#endif

  if (writeAndShowLogs(QStandardPaths::DesktopLocation) ||
      writeAndShowLogs(QStandardPaths::HomeLocation) ||
      writeAndShowLogs(QStandardPaths::TempLocation)) {
    LogHandler::instance()->flushLogs();
    return true;
  }

  logger.warning()
      << "No Desktop, no Home, no Temp folder. Unable to store the log files.";
  return false;
}

bool Utils::writeAndShowLogs(QStandardPaths::StandardLocation location) {
  LogHandler* instance = LogHandler::instance();
  return instance->writeLogsToLocation(location, [](const QString& filename) {
    logger.debug() << "Opening the logFile somehow.";
    UrlOpener::instance()->openUrl(QUrl::fromLocalFile(filename));
  });
}
