/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOCKDAEMON_H
#define MOCKDAEMON_H

#include <QLocalServer>
#include <QString>

#include "daemon.h"
#include "dnsutilsmock.h"
#include "wireguardutilsmock.h"

class MockDaemon final : public Daemon {
  Q_OBJECT

 public:
  MockDaemon(QObject* parent = nullptr);
  MockDaemon(const QString& name, QObject* parent = nullptr);
  ~MockDaemon();

  bool activate(const InterfaceConfig& config) override;
  QString socketPath() const { return m_server.fullServerName(); }

  Obfuscator* createObfuscator(const InterfaceConfig& config) override;

  QStringList getFeatures() const override {
    return QStringList("splitTunnel");
  }

  void setObfuscatorFlags(const QStringList& args) { m_obfuscatorFlags = args; }

  WireguardUtils* wgutils() const override { return m_wgutils; }
  DnsUtils* dnsutils() override { return m_dnsutils; }

 private:
  QString m_socketName;
  QStringList m_obfuscatorFlags;
  QLocalServer m_server;
  DnsUtilsMock* m_dnsutils = nullptr;
  WireguardUtilsMock* m_wgutils = nullptr;
};

#endif  // MOCKDAEMON_H
