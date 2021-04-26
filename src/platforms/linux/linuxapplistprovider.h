/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXAPPLISTPROVIDER_H
#define LINUXAPPLISTPROVIDER_H

#include <applistprovider.h>
#include <QObject>
#include <QProcess>

class LinuxAppListProvider : public AppListProvider {
  Q_OBJECT
 public:
  LinuxAppListProvider(QObject* parent);
  void getApplicationList() override;
};

#endif  // LINUXAPPLISTPROVIDER_H
