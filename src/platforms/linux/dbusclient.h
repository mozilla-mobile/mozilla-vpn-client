/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUSCLIENT_H
#define DBUSCLIENT_H

#include "dbus_interface.h"
#include "firewall_interface.h"

#include <QList>
#include <QObject>

class Server;
class Device;
class Keys;
class IPAddressRange;
class QDBusPendingCallWatcher;

class DBusClient final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DBusClient)

 public:
  DBusClient(QObject* parent);
  ~DBusClient();

  QDBusPendingCallWatcher* version();

  QDBusPendingCallWatcher* activate(
      const Server& server, const Device* device, const Keys* keys,
      const QList<IPAddressRange>& allowedIPAddressRanges);

  QDBusPendingCallWatcher* deactivate();

  QDBusPendingCallWatcher* status();

  QDBusPendingCallWatcher* getLogs();

  QDBusPendingCallWatcher* cleanupLogs();

  QDBusPendingCallWatcher* excludeApp(const QStringList& vpnDisabledApps);

  QDBusPendingCallWatcher* flushApps();

 signals:
  void connected();
  void disconnected();

 private:
  OrgMozillaVpnDbusInterface* m_dbus;
  OrgMozillaVpnFirewallInterface* m_firewall;
};

#endif  // DBUSCLIENT_H
