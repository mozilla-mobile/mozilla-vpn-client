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

#ifdef MVPN_ANDROID
#  include "platforms/android/androidapplistprovider.h"
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
  m_enabledList = new FilteredAppList(this, true);
  m_disabledlist = new FilteredAppList(this, false);

  m_listprovider =
#ifdef MVPN_ANDROID
      new AndroidAppListProvider(this);
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
  QString appID = m_applist.keys().at(index.row());

  switch (role) {
    case AppNameRole:
      return m_applist[appID];
    case AppIdRole:
      return QVariant(appID);
    case AppEnabledRole:
      if (!SettingsHolder::instance()->hasVpnDisabledApps()) {
        // All are enabled then
        return true;
      }
      return !SettingsHolder::instance()->vpnDisabledApps().contains(appID);
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

  int index = m_applist.keys().indexOf(appID);
  dataChanged(createIndex(index, 0), createIndex(index, 0));
}
Q_INVOKABLE void AppPermission::requestApplist() {
  logger.log() << "Request new AppList";
  m_listprovider->getApplicationList();
}

void AppPermission::receiveAppList(const QMap<QString, QString>& applist) {
  if (!m_applist.isEmpty()) {
    // Check the Disabled-List
    auto keys = applist.keys();
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
  m_applist = applist;
  endResetModel();
}

Q_INVOKABLE void AppPermission::protectAll() {
  SettingsHolder::instance()->setVpnDisabledApps(QStringList());
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
};
Q_INVOKABLE void AppPermission::unprotectAll() {
  SettingsHolder::instance()->setVpnDisabledApps(m_applist.keys());
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
};

bool AppPermission::FilteredAppList::filterAcceptsRow(
    int source_row, const QModelIndex& source_parent) const {
  auto index = this->sourceModel()->index(source_row, 0, source_parent);
  if (!index.isValid()) {
    return false;
  }
  auto valueRole = index.data(AppEnabledRole);
  return valueRole.toBool() == mEnabledAppsOnly;
}
