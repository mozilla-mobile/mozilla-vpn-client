/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QDBusContext>
#include <QHash>

#include "apptracker.h"
#include "daemon/daemon.h"
#include "dnsutilslinux.h"
#include "iputilslinux.h"
#include "wireguardutilslinux.h"

class DbusAdaptor;

class DBusService final : public Daemon, protected QDBusContext {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DBusService)
  Q_CLASSINFO("D-Bus Interface", "org.mozilla.vpn.dbus")

 public:
  DBusService(QObject* parent);
  ~DBusService();

  enum AppState { Active, Excluded, Blocked };
  Q_ENUM(AppState)

  void setAdaptor(DbusAdaptor* adaptor);
  void setAppState(const QString& desktopId, AppState state);

  using Daemon::activate;

 public slots:
  bool activate(const QString& jsonConfig);

  bool deactivate(bool emitSignals = true) override;
  QString status();

  QString version();
  QString getLogs();
  void cleanupLogs() { cleanLogs(); }

  QString runningApps();
  bool firewallApp(const QString& appName, const QString& state);
  bool firewallPid(int rootpid, const QString& state);
  bool firewallClear();

 protected:
  WireguardUtils* wgutils() const override { return m_wgutils; }
  bool supportIPUtils() const override { return true; }
  IPUtils* iputils() override;
  bool supportDnsUtils() const override { return true; }
  DnsUtils* dnsutils() override;

 private:
  bool removeInterfaceIfExists();
  bool isCallerAuthorized();
  void dropRootPermissions();

 private slots:
  void appLaunched(const QString& cgroup, const QString& desktopId);
  void appTerminated(const QString& cgroup, const QString& desktopId);

  void userListCompleted(QDBusPendingCallWatcher* call);
  void userCreated(uint uid, const QDBusObjectPath& path);
  void userRemoved(uint uid, const QDBusObjectPath& path);

 private:
  DbusAdaptor* m_adaptor = nullptr;
  WireguardUtilsLinux* m_wgutils = nullptr;
  IPUtilsLinux* m_iputils = nullptr;
  DnsUtilsLinux* m_dnsutils = nullptr;

  AppTracker* m_appTracker = nullptr;
  QHash<QString, AppState> m_excludedApps;
  QHash<QString, AppState> m_excludedCgroups;

  uint m_sessionUid = 0;
};

#endif  // DBUSSERVICE_H
