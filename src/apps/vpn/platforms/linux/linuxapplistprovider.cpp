/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxapplistprovider.h"

#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QString>

#include "leakdetector.h"
#include "logger.h"

constexpr const char* DATA_DIRS_FALLBACK = "/usr/local/share/:/usr/share/";
constexpr const char* CONFIG_DIRS_FALLBACK = "/etc/xdg/autostart/";

namespace {
Logger logger("LinuxAppListProvider");
}

LinuxAppListProvider::LinuxAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MZ_COUNT_CTOR(LinuxAppListProvider);
}

LinuxAppListProvider::~LinuxAppListProvider() {
  MZ_COUNT_DTOR(LinuxAppListProvider);
}

void LinuxAppListProvider::fetchEntries(const QString& dataDir,
                                        QMap<QString, QString>& map) {
  logger.debug() << "Fetch Application list from" << dataDir;

  QDirIterator iter(dataDir, QStringList() << "*.desktop", QDir::Files);
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

    map[fileinfo.absoluteFilePath()] = entry.value("Name").toString();
  }
}

void LinuxAppListProvider::getApplicationList() {
  logger.debug() << "Fetch Application list from Linux desktop";
  QMap<QString, QString> out;

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  QString dataDirs = pe.value("XDG_DATA_DIRS", DATA_DIRS_FALLBACK);
  for (const QString& part : dataDirs.split(":")) {
    fetchEntries(part.trimmed() + "/applications", out);
  }

  if (pe.contains("XDG_DATA_HOME")) {
    fetchEntries(pe.value("XDG_DATA_HOME") + "/applications", out);
  } else if (pe.contains("HOME")) {
    fetchEntries(pe.value("HOME") + "/.local/share/applications", out);
  }

  QMap<QString, QString> autostart;
  QString configDirs = pe.value("XDG_CONFIG_DIRS", CONFIG_DIRS_FALLBACK);
  for (const QString& part : configDirs.split(":")) {
    fetchEntries(part.trimmed() + "/autostart", autostart);
  }

  if (pe.contains("XDG_CONFIG_HOME")) {
    fetchEntries(pe.value("XDG_CONFIG_HOME") + "/autostart", autostart);
  } else if (pe.contains("HOME")) {
    fetchEntries(pe.value("HOME") + "/.config/autostart", autostart);
  }

  for (auto itPathName = autostart.constKeyValueBegin();
       itPathName != autostart.constKeyValueEnd(); ++itPathName) {
    out[itPathName->first] = itPathName->second + " (autostart)";
  }

  emit newAppList(out);
}
