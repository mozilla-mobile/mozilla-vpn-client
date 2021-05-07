/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FIREWALLSERVICE_H
#define FIREWALLSERVICE_H

#include "apptracker.h"
#include "firewall_adaptor.h"
#include "pidtracker.h"

class FirewallService final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FirewallService)
  Q_CLASSINFO("Firewall Interface", "org.mozilla.vpn.firewall")

 public:
  FirewallService(QObject* parent);
  ~FirewallService();

  void initialize(void) { m_pidtracker->initialize(); }

 public slots:
  QString status();
  QString version();
  QString runningApps();
  bool excludeApp(const QStringList& names);
  bool includeApp(const QStringList& names);
  bool flushApps();

 private slots:
  void userListCompleted(QDBusPendingCallWatcher* call);
  void userCreated(uint uid, const QDBusObjectPath& path);
  void userRemoved(uint uid, const QDBusObjectPath& path);
  void pidForked(const QString& name, int parent, int child);
  void pidExited(const QString& name, int pid);
  void appLaunched(const QString& name, uint userid, int rootpid);
  void appTerminate(const QString& name, int rootpid);

 private:
  FirewallAdaptor* m_adaptor;
  QMap<uint, AppTracker*> m_users;
  PidTracker* m_pidtracker;
  QStringList m_excludedApps;

  QString m_defaultCgroup;
  QString m_excludeCgroup;

  static void writeCgroupFile(const QString& path, unsigned long value);
};

#endif  // FIREWALLSERVICE_H
