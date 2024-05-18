/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOCKDAEMON_H
#define MOCKDAEMON_H

#include <QLocalServer>
#include <QString>

#include "daemon/daemon.h"
#include "dnsutilsmock.h"
#include "wireguardutilsmock.h"

class MockDaemon final : public Daemon {
  Q_OBJECT

 public:
  MockDaemon(QObject* parent = nullptr);
  MockDaemon(const QString& name, QObject* parent = nullptr);
  ~MockDaemon();

  static MockDaemon* instance();
  bool activate(const InterfaceConfig& config) override;

 protected:
  WireguardUtils* wgutils() const override { return m_wgutils; }
  DnsUtils* dnsutils() override { return m_dnsutils; }

 private:
  QString m_socketName;
  QLocalServer m_server;
  DnsUtilsMock* m_dnsutils = nullptr;
  WireguardUtilsMock* m_wgutils = nullptr;
};

#endif  // MOCKDAEMON_H
