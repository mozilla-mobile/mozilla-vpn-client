/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdatamigration.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

namespace {
Logger logger(LOG_WINDOWS, "WindowsDataMigration");

void migrateConfigFile(const QString& fileName) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();

  QSettings settings(fileName, QSettings::IniFormat);

  QString token = settings.value("FxA/Token").toString();
  if (!token.isEmpty()) {
    MozillaVPN::instance()->setToken(token);
  }

  QString language = settings.value("Language/PreferredLanguage").toString();
  if (!language.isEmpty()) {
    settingsHolder->setLanguageCode(language);
  }

  bool ipv6Enabled = settings.value("Network/EnableIPv6").toBool();
  settingsHolder->setIpv6Enabled(ipv6Enabled);

  bool captivePortalAlert =
      settings.value("Network/CaptivePortalAlert").toBool();
  settingsHolder->setCaptivePortalAlert(captivePortalAlert);

  bool localNetworkAccess =
      settings.value("Network/AllowLocalDeviceAccess").toBool();
  settingsHolder->setLocalNetworkAccess(localNetworkAccess);
}

void migrateServersFile(const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly)) {
    return;
  }

  MozillaVPN::instance()->setServerList(file.readAll());
}

void migrateFxaFile(const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly)) {
    return;
  }

  MozillaVPN::instance()->accountChecked(file.readAll());
}

void migrateWireguardFile(const QString& fileName) {
  QSettings settings(fileName, QSettings::IniFormat);

  QString privateKey = settings.value("Interface/PrivateKey").toString();
  if (!privateKey.isEmpty()) {
    MozillaVPN::instance()->deviceAdded(Device::currentDeviceName(), QString(),
                                        privateKey);
  }
}

}  // namespace

// static
void WindowsDataMigration::migrate() {
  logger.log() << "Windows Data Migration";

  QStringList paths =
      QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
  for (const QString& path : paths) {
    QDir dir(path);
    if (!dir.exists()) {
      continue;
    }

    QDir mozillaDir(dir.filePath("Mozilla"));
    if (!mozillaDir.exists()) {
      continue;
    }

    QDir fpnDir(mozillaDir.filePath("FirefoxPrivateNetworkVPN"));
    if (!fpnDir.exists()) {
      continue;
    }

    QString confFile(fpnDir.filePath("settings.conf"));
    if (!QFileInfo::exists(confFile)) {
      continue;
    }

    migrateConfigFile(confFile);

    QString serversFile(fpnDir.filePath("servers.json"));
    if (!QFileInfo::exists(serversFile)) {
      continue;
    }

    migrateServersFile(serversFile);

    QString fxaFile(fpnDir.filePath("FirefoxFxAUser.json"));
    if (!QFileInfo::exists(fxaFile)) {
      continue;
    }

    migrateFxaFile(fxaFile);

    QString wireguardFile(fpnDir.filePath("FirefoxPrivateNetworkVPN.conf"));
    if (!QFileInfo::exists(wireguardFile)) {
      continue;
    }

    migrateWireguardFile(wireguardFile);
    break;
  }

  // TODO: default server?
}
