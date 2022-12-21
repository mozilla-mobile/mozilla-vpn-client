/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUSCLIENT_H
#define DBUSCLIENT_H

#include <QHostAddress>
#include <QList>
#include <QObject>

#include "dbus_interface.h"

class Server;
class Device;
class Keys;
class IPAddress;
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
      int hopindex, const QList<IPAddress>& allowedIPAddressRanges,
      const QStringList& excludedAddresses, const QStringList& vpnDisabledApps,
      const QHostAddress& dnsServer);

  QDBusPendingCallWatcher* deactivate();

  QDBusPendingCallWatcher* status();

  QDBusPendingCallWatcher* getLogs();

  QDBusPendingCallWatcher* cleanupLogs();

 signals:
  void connected(const QString& pubkey);
  void disconnected();

 private:
  OrgMozillaVpnDbusInterface* m_dbus;
};

#endif  // DBUSCLIENT_H
