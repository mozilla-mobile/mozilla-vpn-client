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

#include "logger.h"
#include "utils/leakdetector/leakdetector.h"

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
                                        QMap<QString, QString>& map,
                                        const QSet<QString>& desktopEnv) {
  logger.debug() << "Fetch Application list from" << dataDir;

  QDirIterator iter(dataDir, QStringList() << "*.desktop", QDir::Files,
                    QDirIterator::Subdirectories);
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
    if (entry.value("Hidden", QVariant(false)).toBool()) {
      continue;
    }
    if (entry.contains("NotShowIn")) {
      QStringList items =
          entry.value("NotShowIn").toString().trimmed().split(":");
      if (desktopEnv.intersects(QSet<QString>(items.begin(), items.end()))) {
        continue;
      }
    }
    if (entry.contains("OnlyShowIn")) {
      QStringList items =
          entry.value("OnlyShowIn").toString().trimmed().split(":");
      if (!desktopEnv.intersects(QSet<QString>(items.begin(), items.end()))) {
        continue;
      }
    }

    map[fileinfo.absoluteFilePath()] = entry.value("Name").toString();
  }
}

void LinuxAppListProvider::getApplicationList() {
  logger.debug() << "Fetch Application list from Linux desktop";
  QMap<QString, QString> out;

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  QSet<QString> env;
  if (pe.contains("XDG_CURRENT_DESKTOP")) {
    const QStringList parts = pe.value("XDG_CURRENT_DESKTOP").split(":");
    env += QSet<QString>(parts.begin(), parts.end());
  }

  QString dataDirs = pe.value("XDG_DATA_DIRS", DATA_DIRS_FALLBACK);
  for (const QString& part : dataDirs.split(":")) {
    fetchEntries(part.trimmed() + "/applications", out, env);
  }

  if (pe.contains("XDG_DATA_HOME")) {
    fetchEntries(pe.value("XDG_DATA_HOME") + "/applications", out, env);
  } else if (pe.contains("HOME")) {
    fetchEntries(pe.value("HOME") + "/.local/share/applications", out, env);
  }

  QMap<QString, QString> autostart;
  QString configDirs = pe.value("XDG_CONFIG_DIRS", CONFIG_DIRS_FALLBACK);
  for (const QString& part : configDirs.split(":")) {
    fetchEntries(part.trimmed() + "/autostart", autostart, env);
  }

  if (pe.contains("XDG_CONFIG_HOME")) {
    fetchEntries(pe.value("XDG_CONFIG_HOME") + "/autostart", autostart, env);
  } else if (pe.contains("HOME")) {
    fetchEntries(pe.value("HOME") + "/.config/autostart", autostart, env);
  }

  for (auto itPathName = autostart.constKeyValueBegin();
       itPathName != autostart.constKeyValueEnd(); ++itPathName) {
    out[itPathName->first] = itPathName->second + " (autostart)";
  }

  emit newAppList(out);
}
