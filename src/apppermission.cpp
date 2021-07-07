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
#else
#  include "platforms/dummy/dummyapplistprovider.h"
#endif

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

Q_INVOKABLE void AppPermission::flip(const QString& appID) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->hasVpnDisabledApp(appID)) {
    logger.log() << "Enabled --" << appID << " for VPN";
    settingsHolder->removeVpnDisabledApp(appID);
  } else {
    logger.log() << "Disabled --" << appID << " for VPN";
    settingsHolder->addVpnDisabledApp(appID);
  }

  int index = m_applist.indexOf(AppDescription(appID));
  dataChanged(createIndex(index, 0), createIndex(index, 0));
}

Q_INVOKABLE void AppPermission::requestApplist() {
  logger.log() << "Request new AppList";
  m_listprovider->getApplicationList();
}

void AppPermission::receiveAppList(const QMap<QString, QString>& applist) {
  auto keys = applist.keys();
  if (!m_applist.isEmpty()) {
    // Check the Disabled-List
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    foreach (QString blockedAppID, settingsHolder->vpnDisabledApps()) {
      if (!keys.contains(blockedAppID)) {
        logger.log() << "Removed obsolete appid" << blockedAppID;
        settingsHolder->removeVpnDisabledApp(blockedAppID);
      }
    }
  }
  beginResetModel();
  logger.log() << "Recived new Applist -- Entrys: " << applist.size();
  m_applist.clear();
  for (auto id : keys) {
    m_applist.append(AppDescription(id, applist[id]));
  }
  std::sort(m_applist.begin(), m_applist.end());
  endResetModel();
}

Q_INVOKABLE void AppPermission::protectAll() {
  SettingsHolder::instance()->setVpnDisabledApps(QStringList());
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
};
Q_INVOKABLE void AppPermission::unprotectAll() {
  QStringList allAppIds;
  for (auto app : m_applist) {
    allAppIds.append(app.id);
  }
  SettingsHolder::instance()->setVpnDisabledApps(allAppIds);
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
};
