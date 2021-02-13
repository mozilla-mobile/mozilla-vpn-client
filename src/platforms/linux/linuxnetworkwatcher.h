/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXNETWORKWATCHER_H
#define LINUXNETWORKWATCHER_H

#include "networkwatcherimpl.h"

#include <QMap>
#include <QVariant>

class LinuxNetworkWatcher final : public NetworkWatcherImpl {
  Q_OBJECT

 public:
  LinuxNetworkWatcher(QObject* parent);
  ~LinuxNetworkWatcher();

  void initialize() override;

  void start() override;

 private slots:
  void propertyChanged(QString interface, QVariantMap properties,
                       QStringList list);

  void checkDevices();

 private:
  // We collect the list of DBus wifi network device paths during the
  // initialization. When a property of them changes, we check if the access
  // point is active and unsecure.
  QStringList m_devicePaths;
};

#endif  // LINUXNETWORKWATCHER_H
