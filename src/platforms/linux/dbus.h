/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUS_H
#define DBUS_H

#include "dbus_interface.h"

#include <QList>
#include <QObject>

class Server;
class Device;
class Keys;
class IPAddressRange;
class QDBusPendingCallWatcher;

class DBus final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DBus)

 public:
  DBus(QObject* parent);
  ~DBus();

  QDBusPendingCallWatcher* version();

  QDBusPendingCallWatcher* activate(
      const Server& server, const Device* device, const Keys* keys,
      const QList<IPAddressRange>& allowedIPAddressRanges);

  QDBusPendingCallWatcher* deactivate();

  QDBusPendingCallWatcher* status();

  QDBusPendingCallWatcher* logs();

 signals:
  void connected();
  void disconnected();

 private:
  OrgMozillaVpnDbusInterface* m_dbus;
};

#endif  // DBUS_H
