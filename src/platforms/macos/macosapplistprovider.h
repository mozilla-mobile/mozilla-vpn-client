/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSAPPLISTPROVIDER_H
#define MACOSAPPLISTPROVIDER_H

#include <applistprovider.h>

#include <QObject>
#include <QProcess>

class MacOSAppListProvider final : public AppListProvider {
  Q_OBJECT
 public:
  explicit MacOSAppListProvider(QObject* parent);
  ~MacOSAppListProvider();
  void getApplicationList() override;

 private:
  void fetchEntries(const QString& appDir, QList<AppDescription>& out);
};

#endif  // MACOSAPPLISTPROVIDER_H
