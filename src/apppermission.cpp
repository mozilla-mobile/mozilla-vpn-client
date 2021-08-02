/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apppermission.h"
#include "applistprovider.h"
#include "logger.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include <QApplication>
#include <QVector>
#include "settingsholder.h"

#if defined(MVPN_ANDROID)
#  include "platforms/android/androidapplistprovider.h"
#elif defined(MVPN_LINUX)
#  include "platforms/linux/linuxapplistprovider.h"
#elif defined(MVPN_WINDOWS)
#  include "platforms/windows/windowsapplistprovider.h"
#else
#  include "platforms/dummy/dummyapplistprovider.h"
#endif

#include <QFileDialog>

namespace {
Logger logger(LOG_MAIN, "AppPermission");
AppPermission* s_instance = nullptr;
}  // namespace

AppPermission::AppPermission(QObject* parent) : QAbstractListModel(parent) {
  MVPN_COUNT_CTOR(AppPermission);
  Q_ASSERT(!s_instance);
  s_instance = this;

  m_listprovider =
#if defined(MVPN_ANDROID)
      new AndroidAppListProvider(this);
#elif defined(MVPN_LINUX)
      new LinuxAppListProvider(this);
#elif defined(MVPN_WINDOWS)
      new WindowsAppListProvider(this);
#else
      new DummyAppListProvider(this);
#endif

  connect(m_listprovider, &AppListProvider::newAppList, this,
          &AppPermission::receiveAppList);
}
AppPermission::~AppPermission() {
  MVPN_COUNT_DTOR(AppPermission);
  Q_ASSERT(s_instance = this);
  s_instance = nullptr;
}

AppPermission* AppPermission::instance() {
  if (s_instance == nullptr) {
    new AppPermission(qApp);
  }
  Q_ASSERT(s_instance);
  return s_instance;
}

QHash<int, QByteArray> AppPermission::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[AppNameRole] = "appName";
  roles[AppIdRole] = "appID";
  roles[AppEnabledRole] = "appIsEnabled";
  return roles;
}
int AppPermission::rowCount(const QModelIndex&) const {
  return m_applist.size();
}
QVariant AppPermission::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  const AppDescription& app = m_applist.at(index.row());

  switch (role) {
    case AppNameRole:
      return app.name;
    case AppIdRole:
      return QVariant(app.id);
    case AppEnabledRole:
      if (!SettingsHolder::instance()->hasVpnDisabledApps()) {
        // All are enabled then
        return true;
      }
      return !SettingsHolder::instance()->vpnDisabledApps().contains(app.id);
    default:
      return QVariant();
  }
}

void AppPermission::flip(const QString& appID) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->hasVpnDisabledApp(appID)) {
    logger.debug() << "Enabled --" << appID << " for VPN";
    settingsHolder->removeVpnDisabledApp(appID);
  } else {
    logger.debug() << "Disabled --" << appID << " for VPN";
    settingsHolder->addVpnDisabledApp(appID);
  }

  int index = m_applist.indexOf(AppDescription(appID));
  dataChanged(createIndex(index, 0), createIndex(index, 0));
}

void AppPermission::requestApplist() {
  logger.debug() << "Request new AppList";
  m_listprovider->getApplicationList();
}

void AppPermission::receiveAppList(const QMap<QString, QString>& applist) {
  auto keys = applist.keys();
  if (!m_applist.isEmpty()) {
    // Check the Disabled-List
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    foreach (QString blockedAppId, settingsHolder->vpnDisabledApps()) {
      if (!m_listprovider->isValidAppId(blockedAppId)) {
        // In case the AppID is no longer valid we don't need to keep it
        logger.debug() << "Removed obsolete appid" << blockedAppId;
        settingsHolder->removeVpnDisabledApp(blockedAppId);
      } else if (!keys.contains(blockedAppId)) {
        // In case the AppID is valid but not in our applist, we need to create
        // an entry
        logger.debug() << "Added missing appid" << blockedAppId;
        m_applist.append(AppDescription(blockedAppId, applist[blockedAppId]));
      }
    }
  }
  beginResetModel();
  logger.debug() << "Recived new Applist -- Entrys: " << applist.size();
  m_applist.clear();
  for (auto id : keys) {
    m_applist.append(AppDescription(id, applist[id]));
  }
  std::sort(m_applist.begin(), m_applist.end());
  endResetModel();
}

void AppPermission::protectAll() {
  logger.debug() << "Protected all";

  SettingsHolder::instance()->setVpnDisabledApps(QStringList());
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
};

void AppPermission::unprotectAll() {
  logger.debug() << "Unprotected all";

  QStringList allAppIds;
  for (auto app : m_applist) {
    allAppIds.append(app.id);
  }
  SettingsHolder::instance()->setVpnDisabledApps(allAppIds);
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
}

void AppPermission::openFilePicker() {
  logger.debug() << "File picker required";

  QFileDialog fp(nullptr, qtTrId("vpn.protectSelectedApps.addApplication"));

  QStringList locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (!locations.isEmpty()) {
    fp.setDirectory(locations.first());
  }

  fp.setFilter(QDir::Executable | QDir::Files | QDir::NoDotAndDotDot);
  fp.setFileMode(QFileDialog::ExistingFile);

  if (!fp.exec()) {
    logger.error() << "File picker exection aborted";
    return;
  }

  QStringList fileNames = fp.selectedFiles();
  if (fileNames.isEmpty()) {
    logger.warning() << "File picker - no selection";
    return;
  }

  logger.debug() << "Selection:" << fileNames;
  Q_ASSERT(fileNames.length() == 1);

  Q_ASSERT(m_listprovider);
  m_listprovider->addApplication(fileNames[0]);
}
