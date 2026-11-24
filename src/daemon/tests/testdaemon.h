/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TESTDAEMON_H
#define TESTDAEMON_H

#include <QByteArray>
#include <QLocalSocket>
#include <QObject>
#include <QString>

#include "daemon.h"
#include "mock/dnsutilsmock.h"
#include "mock/wireguardutilsmock.h"
#include "obfuscator/qprocessobfuscator.h"

class QJsonObject;

// Base test case that starts and stops a mock daemon
class TestDaemon : public QObject {
  Q_OBJECT

 public:
  void send(const QJsonObject& obj);
  bool recv(QJsonObject& obj, int timeout = 30000);

  static QString exampleKey(const char* prefix, char fill = 'A');
  static InterfaceConfig exampleConfig(char seed = 'A');

  static QList<const QMetaObject*> testcases();

  QProcessObfuscator* obfuscator() const {
    return qobject_cast<QProcessObfuscator*>(m_daemon->m_obfuscator.get());
  }

 protected slots:
  void init();
  void cleanup();

 protected:
  QScopedPointer<Daemon> m_daemon;
  QScopedPointer<QLocalSocket> m_socket;
  DnsUtilsMock* m_dnsutils = nullptr;
  WireguardUtilsMock* m_wgutils = nullptr;

  QStringList m_connectedEvents;
  int m_disconnectedEventCount;
  QList<DaemonError> m_failureEvents;
};

#endif  // TESTDAEMON_H
