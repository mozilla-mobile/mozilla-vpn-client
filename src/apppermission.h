/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPPERMISSION_H
#define APPPERMISSION_H

#include <QObject>
#include <QAbstractListModel>
#include "applistprovider.h"

class AppPermission final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppPermission)

 public:
  ~AppPermission();

  enum AppPermissionRoles {
    AppNameRole,
    AppIdRole,
    AppEnabledRole,
  };

  static AppPermission* instance();
  // Enables/Disabled the Given App ID for the vpn
  Q_INVOKABLE void flip(const QString& appID);
  // Is called from QML if the List is opened
  Q_INVOKABLE void requestApplist();

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;
 signals:
  void readyChanged();
 private slots:
  void receiveAppList(const QMap<QString, QString>& applist);

 private:
  AppPermission(QObject* parent);
  AppListProvider* m_listprovider = nullptr;
  QStringList m_disabledAppList;
  QMap<QString, QString> m_applist;
 signals:
};

#endif // APPPERMISSION_H
