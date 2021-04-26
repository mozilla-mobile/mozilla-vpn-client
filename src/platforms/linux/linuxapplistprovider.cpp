/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxapplistprovider.h"
#include "leakdetector.h"

#include <QProcess>
#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QSettings>

#include "logger.h"
#include "leakdetector.h"

constexpr const char* DESKTOP_ENTRY_LOCATION = "/usr/share/applications/";

namespace {
Logger logger(LOG_CONTROLLER, "LinuxAppListProvider");
}

LinuxAppListProvider::LinuxAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MVPN_COUNT_CTOR(LinuxAppListProvider);
}

void LinuxAppListProvider::getApplicationList() {
  logger.log() << "Fetch Application list from Linux desktop";

  QMap<QString, QString> out;
  QDirIterator iter(DESKTOP_ENTRY_LOCATION, QStringList() << "*.desktop", QDir::Files);
  while (iter.hasNext()) {
    QFileInfo fileinfo(iter.next());
    QSettings entry(fileinfo.filePath(), QSettings::IniFormat);
    entry.beginGroup("Desktop Entry");

    /* Filter out everything except visible applications. */
    if (entry.value("Type").toString() != "Application") {
      continue;
    }
    if (entry.value("NoDisplay", QVariant(false)).toBool()) {
      continue;
    }

    out[fileinfo.fileName()] = entry.value("Name").toString();
  }

  emit newAppList(out);
}
