/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPPERMISSION_H
#define APPPERMISSION_H

#include <QAbstractListModel>
#include <QObject>
#include <QPair>

#include "applistprovider.h"

class AppPermission final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppPermission)

  Q_PROPERTY(bool containsSystemApps READ containsSystemApps NOTIFY
                 containsSystemAppsChanged)

 public:
  ~AppPermission();

  bool containsSystemApps() const;

  enum AppPermissionRoles {
    AppNameRole,
    AppIdRole,
    AppEnabledRole,
    AppIsSystemAppRole,
  };

  static AppPermission* instance();
  static void mock();

  // Returns count of disabled apps
  Q_INVOKABLE int disabledAppCount();
  // Enables/Disabled the Given App ID for the vpn
  Q_INVOKABLE void flip(const QString& appID);
  // Is called from QML if the List is opened
  Q_INVOKABLE void requestApplist();

  // Add all Apps to the Disabled App List
  Q_INVOKABLE void protectAll();
  // Remove all Apps from the Disabled App List
  Q_INVOKABLE void unprotectAll();

  Q_INVOKABLE void openFilePicker();

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;
 signals:
  void readyChanged();
  void containsSystemAppsChanged();
  void notification(const QString& type, const QString& message,
                    const QString& actionMessage = "");
 private slots:
  void receiveAppList(const QList<AppListProvider::AppDescription>& applist);

 private:
  explicit AppPermission(AppListProvider* provider, QObject* parent = nullptr);
  struct MissingAppList;
  MissingAppList retrieveMissingApps(
      QList<AppListProvider::AppDescription> alreadyFoundApps);

  AppListProvider* m_listprovider = nullptr;
  QList<AppListProvider::AppDescription> m_applist;
};

#endif  // APPPERMISSION_H
