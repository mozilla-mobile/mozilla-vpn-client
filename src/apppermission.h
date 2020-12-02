/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPPERMISSION_H
#define APPPERMISSION_H

#include <QObject>
#include <QAbstractListModel>

class AppPermission : public QAbstractListModel
{
    Q_OBJECT

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

    // QAbstractListModel methods

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex&) const override;

    QVariant data(const QModelIndex& index, int role) const override;
private:
     AppPermission();

signals:

};

#endif // APPPERMISSION_H
