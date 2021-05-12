/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPPERMISSION_H
#define APPPERMISSION_H

#include <QObject>
#include <QPair>
#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include "applistprovider.h"

class AppPermission final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppPermission)

  Q_PROPERTY(QSortFilterProxyModel* enabledApps MEMBER m_enabledList CONSTANT)
  Q_PROPERTY(QSortFilterProxyModel* disabledApps MEMBER m_disabledlist CONSTANT)

 public:
  ~AppPermission();

  enum AppPermissionRoles {
    AppNameRole,
    AppIdRole,
    AppEnabledRole,
  };

  class AppDescription {
   public:
    AppDescription(const QString& appId, const QString& appName) {
      name = appName;
      id = appId;
    };
    QString name;
    QString id;

    bool operator<(const AppDescription& other) const {
      return name.compare(other.name, Qt::CaseInsensitive) < 0;
    }
    bool operator>(const AppDescription& other) const {
      return name.compare(other.name, Qt::CaseInsensitive) > 0;
    }
    bool operator==(const AppDescription& other) const {
      return id == other.id;
    }
    bool operator==(const QString& appId) const { return id == appId; }
  };

  static AppPermission* instance();
  // Enables/Disabled the Given App ID for the vpn
  Q_INVOKABLE void flip(const QString& appID);
  // Is called from QML if the List is opened
  Q_INVOKABLE void requestApplist();

  // Add all Apps to the Disabled App List
  Q_INVOKABLE void protectAll();
  // Remove all Apps from the Disabled App List
  Q_INVOKABLE void unprotectAll();

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
  QList<AppDescription> m_applist;

  // Sublist of AppPermission, can either include all Enabled or Disabled apps
  class FilteredAppList : public QSortFilterProxyModel {
   public:
    bool mEnabledAppsOnly;
    FilteredAppList(AppPermission* parent, bool enabledAppsOnly)
        : QSortFilterProxyModel(parent), mEnabledAppsOnly(enabledAppsOnly) {
      this->setSourceModel(parent);
    };
    bool filterAcceptsRow(int source_row,
                          const QModelIndex& source_parent) const override;
  };
  QSortFilterProxyModel* m_enabledList;
  QSortFilterProxyModel* m_disabledlist;
};

#endif  // APPPERMISSION_H
