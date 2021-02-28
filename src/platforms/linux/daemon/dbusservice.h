/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include "daemon/daemon.h"
#include "wgutilslinux.h"

class DbusAdaptor;

class DBusService final : public Daemon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DBusService)
  Q_CLASSINFO("D-Bus Interface", "org.mozilla.vpn.dbus")

 public:
  DBusService(QObject* parent);
  ~DBusService();

  void setAdaptor(DbusAdaptor* adaptor);

  bool removeInterfaceIfExists();

  using Daemon::activate;

 public slots:
  bool activate(const QString& jsonConfig);

  bool deactivate(bool emitSignals = true) override;
  QString status();

  QString version();
  QString getLogs();

 protected:
  bool run(Op op, const Config& config) override;

  bool supportServerSwitching(const Config& config) const override;
  bool switchServer(const Config& config) override;

  bool supportWGUtils() const override { return true; }
  WireguardUtilsLinux* wgutils() override;

  QByteArray getStatus() override;

 private:
  DbusAdaptor* m_adaptor = nullptr;
  WireguardUtilsLinux* m_wgutils = nullptr;
};

#endif  // DBUSSERVICE_H
