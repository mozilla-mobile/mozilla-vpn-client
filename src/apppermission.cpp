/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apppermission.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include <QVector>
namespace {
Logger logger(LOG_MAIN, "AppPermission");
AppPermission* s_instance = nullptr;
}

AppPermission::AppPermission()
{
//?    MVPN_COUNT_CTOR(AppPermission);

    Q_ASSERT(!s_instance);
    s_instance = this;
}
AppPermission::~AppPermission(){
//?    MVPN_COUNT_DTOR(AppPermission);

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

// ListModel Functions
QHash<int, QByteArray> AppPermission::roleNames() const {
      QHash<int, QByteArray> roles;
      roles[AppNameRole] ="appName";
      roles[AppIdRole] ="appID";
      roles[AppEnabledRole] ="appIsEnabled";
      return roles;

}
int AppPermission::rowCount(const QModelIndex&) const {
    return 10;

}
QVariant AppPermission::data(const QModelIndex& index, int role) const {

    QString appID ="com.lul.hi";
    Q_UNUSED(index);


    switch(role){
    case AppNameRole:
        return QVariant("LUL APPNAME");
    case AppIdRole:
        return QVariant(appID);
    case AppEnabledRole:
        if(!SettingsHolder::instance()->hasVpnDisabledApps()){
            // All are enabled then
            return true;
        }
        return !SettingsHolder::instance()->vpnDisabledApps().contains(appID);
    }



    return QVariant();
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
