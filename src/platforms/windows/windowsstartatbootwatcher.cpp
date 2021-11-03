/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsstartatbootwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

#include <QCoreApplication>
#include <QSettings>
#include <QDir>
namespace {
Logger logger(LOG_WINDOWS, "WindowsStartAtBootWatcher");
}

WindowsStartAtBootWatcher::WindowsStartAtBootWatcher(bool startAtBoot) {
  MVPN_COUNT_CTOR(WindowsStartAtBootWatcher);

  logger.debug() << "StartAtBoot watcher";
  startAtBootChanged(startAtBoot);
}

WindowsStartAtBootWatcher::~WindowsStartAtBootWatcher() {
  MVPN_COUNT_DTOR(WindowsStartAtBootWatcher);
}

void WindowsStartAtBootWatcher::startAtBootChanged(const bool& startAtBoot) {
  logger.debug() << "StartAtBoot changed:" << startAtBoot;
  QSettings settings(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
  if (startAtBoot) {
    settings.setValue(
        "Mozilla_VPN",
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
  } else {
    settings.remove("Mozilla_VPN");
  }
  settings.sync();
}
