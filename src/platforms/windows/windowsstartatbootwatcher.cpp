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

WindowsStartAtBootWatcher::WindowsStartAtBootWatcher() {
  MVPN_COUNT_CTOR(WindowsStartAtBootWatcher);

  logger.debug() << "StartAtBoot watcher";

  connect(SettingsHolder::instance(), &SettingsHolder::startAtBootChanged, this,
          &WindowsStartAtBootWatcher::startAtBootChanged);

  startAtBootChanged();
}

WindowsStartAtBootWatcher::~WindowsStartAtBootWatcher() {
  MVPN_COUNT_DTOR(WindowsStartAtBootWatcher);
}

void WindowsStartAtBootWatcher::startAtBootChanged() {
  bool startAtBoot = SettingsHolder::instance()->startAtBoot();

  logger.debug() << "StartAtBoot changed:" << startAtBoot;
  QSettings settings(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
  if (startAtBoot) {
    settings.setValue("Mozilla_VPN",
                      QString("\"%1\" ui -m -s")
                          .arg(QDir::toNativeSeparators(
                              QCoreApplication::applicationFilePath())));
  } else {
    settings.remove("Mozilla_VPN");
  }
  settings.sync();
}
