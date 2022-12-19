/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSAPPLISTPROVIDER_H
#define WINDOWSAPPLISTPROVIDER_H

#include <QFileInfo>
#include <QObject>

#include "applistprovider.h"

class WindowsAppListProvider final : public AppListProvider {
  Q_OBJECT
 public:
  WindowsAppListProvider(QObject* parent);
  ~WindowsAppListProvider();
  void getApplicationList() override;
  QString getAppName(const QString& appId) override;

  bool isValidAppId(const QString& appId) override;

  void addApplication(const QString& appPath) override;

 private:
  void readLinkFiles(const QString& path, QMap<QString, QString>& out);
  static QStringList getUninstallerList();

  bool isUninstaller(const QFileInfo& file);
  QStringList m_uninstallerCommands;
};

#endif  // WINDOWSAPPLISTPROVIDER_H
