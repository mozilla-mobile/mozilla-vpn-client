/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPPERMISSION_H
#define APPPERMISSION_H

#include <QObject>
#include <QAbstractListModel>
#include "applistprovider.h"

class AppPermission : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AppPermission)
    Q_PROPERTY(bool listReady READ listReady NOTIFY readyChanged)

public:
    ~AppPermission();

    enum AppPermissionRoles {
       AppNameRole,
       AppIdRole,
       AppEnabledRole,
     };


    static AppPermission* instance();
    // Enables/Disabled the Given App ID for the vpn
    Q_INVOKABLE void flip(QString appID);
    bool listReady() const { return m_ready; }

    // QAbstractListModel methods

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex&) const override;

    QVariant data(const QModelIndex& index, int role) const override;
signals:
    void readyChanged();
private slots:
 void reciveAppList(QMap<QString,QString> applist);

private:
     AppPermission();
     AppListProvider* m_listprovider;
     QMap<QString,QString> m_applist;
     bool m_ready =false;
signals:

};

#endif // APPPERMISSION_H
