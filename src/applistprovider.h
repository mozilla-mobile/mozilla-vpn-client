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
  struct AppListEntry {
    // The User Readable Name of the Application
    QString label;
    // True if the Application is a System App
    // in that case it's hidden by default.
    bool isSystemApp;
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
  void newAppList(const QMap<AppId, AppListEntry>& applist);
};

#endif  // APPLISTPROVIDER_H
