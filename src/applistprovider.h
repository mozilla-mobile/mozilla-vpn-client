/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPLISTPROVIDER_H
#define APPLISTPROVIDER_H

#include <QMap>
#include <QObject>
#include <QQuickImageProvider>

class AppListProvider : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppListProvider)

 public:
  AppListProvider(QObject* parent) : QObject(parent) {}

  virtual ~AppListProvider() = default;

  // A Stable Identifier for an Application - should not change between
  // invokations.
  typedef QString AppId;
  class AppDescription {
   public:
    AppDescription(const QString& appId, const QString& appName = "",
                   bool isSystem = false) {
      name = appName;
      id = appId;
      isSystemApp = isSystem;
    };
    QString name;
    AppId id;
    bool isSystemApp;

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
  // Requests a fresh Application List
  // Impl should emit newAppList signal when done.
  virtual void getApplicationList() = 0;

  virtual void addApplication(const QString& appPath) { Q_UNUSED(appPath); }

  // Returns true if the AppId Exists and is valid
  virtual bool isValidAppId(const AppId& appId) {
    Q_UNUSED(appId);
    return true;
  }
  // Returns the Name of an AppId
  virtual QString getAppName(const AppId& appId) { return appId; };
 signals:
  // Should be emitted when the new Application List is Ready
  // QMap key should be the identifier that the controller can
  // use on activation.
  // QMap Value should be a User readable Name of the app
  void newAppList(const QList<AppDescription>& applist);
};

#endif  // APPLISTPROVIDER_H
