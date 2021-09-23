/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsapplistprovider.h"
#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"
#include "windowscommons.h"

#include <QSettings>
#include <QString>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <Windows.h>

#include "windowsappimageprovider.h"

namespace {
const QString REG_UNINSTALLER =
    R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\)";
Logger logger(LOG_WINDOWS, "AppListProvider");
}  // namespace

WindowsAppListProvider::WindowsAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MVPN_COUNT_CTOR(WindowsAppListProvider);
}

WindowsAppListProvider::~WindowsAppListProvider() {
  MVPN_COUNT_DTOR(WindowsAppListProvider);
}

/**
 * @brief Assembles an Application list through reading all links in common
 * locations
 */
void WindowsAppListProvider::getApplicationList() {
  QMap<QString, QString> appList;

  readLinkFiles("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs",
                appList);
  readLinkFiles(
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
          "\\..\\Microsoft\\Windows\\Start Menu\\Programs",
      appList);
  readLinkFiles(
      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
      appList);
  emit newAppList(appList);
}

void WindowsAppListProvider::addApplication(const QString& appPath) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QStringList applist = settingsHolder->missingApps();
  if (!applist.contains(appPath)) {
    appList.append(appPath);
    settingsHolder->setMissingApps(applist);
  }

  getApplicationList();
}

QString WindowsAppListProvider::getAppName(const QString& appId) {
  return QFileInfo(appId).fileName();
}

bool WindowsAppListProvider::isValidAppId(const QString& appId) {
  const QFileInfo info(appId);
  return info.exists() && info.isExecutable() &&
         WindowsAppImageProvider::hasImage(appId);
}
/**
 * @brief Reads all .lnk's in a Dir, filters them and Puts them into a QMap
 * @param path - Directory Path to read
 * @param out - QMap which valid links should be put into
 */
void WindowsAppListProvider::readLinkFiles(const QString& path,
                                           QMap<QString, QString>& out) {
  QFileInfo self(WindowsCommons::getCurrentPath());
  logger.debug() << "Read -> " << path;
  QDirIterator it(path, QStringList() << "*.lnk", QDir::Files,
                  QDirIterator::Subdirectories);
  const auto oldCount = out.count();
  while (it.hasNext()) {
    QFileInfo link(it.next());
    if (!link.isShortcut()) {
      // Only care for links
      continue;
    }
    QFileInfo target(link.symLinkTarget());
    if (!target.isExecutable()) {
      // 1:  We only care for .exe
      logger.debug() << "Skip -> " << link.baseName()
                     << target.absoluteFilePath();
      continue;
    }
    if (target.fileName() == self.fileName()) {
      // 2: Dont Include ourselves :)
      logger.debug() << "Skip -> " << link.baseName()
                     << target.absoluteFilePath();
      continue;
    }
    if (target.path().toUpper().startsWith("C:/WINDOWS")) {
      // 3: Don't include windows links like cmd/ps
      logger.debug() << "Skip -> " << link.baseName()
                     << target.absoluteFilePath();
      continue;
    }
    if (isUninstaller(target)) {
      // 4: Don't include obvious uninstallers
      logger.debug() << "Skip -> " << link.baseName()
                     << target.absoluteFilePath();
      continue;
    }
    if (!WindowsAppImageProvider::hasImage(target.absoluteFilePath())) {
      // 5: Don't include apps without an icon
      logger.debug() << "Skip -> " << link.baseName()
                     << target.absoluteFilePath();
      continue;
    }
    logger.debug() << "Add -> " << link.baseName() << target.absoluteFilePath();
    out.insert(target.absoluteFilePath(), link.baseName());
  }
  logger.debug() << " Added: " << out.count() - oldCount;
}

QStringList WindowsAppListProvider::getUninstallerList() {
  const QSettings uninstallers =
      QSettings(REG_UNINSTALLER, QSettings::NativeFormat);

  auto childGroups = uninstallers.childGroups();
  QStringList out;
  for (const QString& key : childGroups) {
    QSettings installedProgram =
        QSettings(REG_UNINSTALLER + key, QSettings::NativeFormat);
    if (installedProgram.contains("UninstallString")) {
      out.append(installedProgram.value("UninstallString").toString());
    }
  }
  return out;
}

bool WindowsAppListProvider::isUninstaller(const QFileInfo& file) {
  if (m_uninstallerCommands.size() == 0) {
    m_uninstallerCommands = getUninstallerList();
  }

  const auto nativePath = QDir::toNativeSeparators(file.absoluteFilePath());

  // m_uninstallerCommands contains a list of cmd commands that are called
  // if the user wants to uninstall the program
  // we'll check if the full path of the .exe is in one of those commands, it's
  // most likely an uninstaller.
  for (const QString& command : m_uninstallerCommands) {
    if (command.contains(nativePath)) {
      return true;
    }
  }

  // Not all uninstallers register themselfs there and instead
  // just add an uninstaller .lnk into the programm menu
  // lets ignore .exe with the format ABCuninstaller.exe
  logger.debug() << file.fileName().toLower();
  if (file.fileName().toLower().contains("uninstall")) {
    return true;
  };
  // Also ignore the favorite name for uninstallers
  if (file.fileName() == "unins000.exe") {
    return true;
  }

  return false;
}
