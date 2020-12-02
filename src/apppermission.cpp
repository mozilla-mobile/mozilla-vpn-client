/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apppermission.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "applistprovider.h"
#include <QVector>

// TODO :: #ifdef
#include "platforms/dummy/dummyapplistprovider.h"
// #endif


namespace {
Logger logger(LOG_MAIN, "AppPermission");
AppPermission* s_instance = nullptr;
}

AppPermission::AppPermission()
{
 //   MVPN_COUNT_CTOR(AppPermission);
    Q_ASSERT(!s_instance);
    s_instance = this;
    // Todo: ifdef for plattforms
    m_listprovider = new DummyAppListProvider();
    // #endif
    connect(m_listprovider, &AppListProvider::newAppList, this,
            &AppPermission::reciveAppList);

    m_listprovider->getApplicationList();

}
AppPermission::~AppPermission(){
 //   MVPN_COUNT_DTOR(AppPermission);

    Q_ASSERT(s_instance = this);
    s_instance = nullptr;
}

AppPermission* AppPermission::instance(){
    if(s_instance== nullptr){
        new AppPermission();
    }
    Q_ASSERT(s_instance);
    return s_instance;
}

QHash<int, QByteArray> AppPermission::roleNames() const {
      QHash<int, QByteArray> roles;
      roles[AppNameRole] ="appName";
      roles[AppIdRole] ="appID";
      roles[AppEnabledRole] ="appIsEnabled";
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


    switch(role){
    case AppNameRole:
        return m_applist[appID];
    case AppIdRole:
        return QVariant(appID);
    case AppEnabledRole:
        if(!SettingsHolder::instance()->hasVpnDisabledApps()){
            // All are enabled then
            return true;
        }
        return !SettingsHolder::instance()->vpnDisabledApps().contains(appID);
     default:
       return QVariant();
    }
}

Q_INVOKABLE void  AppPermission::flip(QString appID){
    beginResetModel();
    QStringList list;
    if(SettingsHolder::instance()->hasVpnDisabledApps()){
        list = SettingsHolder::instance()->vpnDisabledApps();
    }
    if(list.contains(appID)){
        logger.log() << "Enabled --" << appID << " for VPN";
        list.removeAll(appID);
    }else{
        logger.log() << "Disabled --" << appID << " for VPN";
        list.append(appID);
    }
    SettingsHolder::instance()->setVpnDisabledApps(list);
   endResetModel(); // Todo:: not use Reset Model - toooo slow.
}

void AppPermission::reciveAppList(QMap<QString,QString> applist){
    if(!m_ready){
        m_ready = true;
        emit readyChanged();
    }
     beginResetModel();
     logger.log() << "Recived new Applist -- Entrys: " << applist.size();
     m_applist = applist;
     endResetModel();
}
