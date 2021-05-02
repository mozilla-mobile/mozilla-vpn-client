/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FIREWALLSERVICE_H
#define FIREWALLSERVICE_H

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
  void trackApp(const QString& name, int rootpid);

 public slots:
  QString status();
  QString version();
  void excludeApp(const QStringList& names);
  void includeApp(const QStringList& names);

 private slots:
  void pidForked(const QString& name, int parent, int child);
  void pidExited(const QString& name, int pid);
  void appTerminate(const QString& name, int rootpid);

 private:
  FirewallAdaptor* m_adaptor;
  PidTracker* m_pidtracker;
  QStringList m_excludedApps;
};

#endif  // FIREWALLSERVICE_H
