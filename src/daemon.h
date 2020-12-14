/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DAEMON_H
#define DAEMON_H

#include <QObject>

class Daemon : public QObject {
  Q_OBJECT

 public:
  enum Op {
    Up,
    Down,
  };

  struct Config {
    QString m_privateKey;
    QString m_deviceIpv4Address;
    QString m_deviceIpv6Address;
    QString m_serverIpv4Gateway;
    QString m_serverIpv6Gateway;
    QString m_serverPublicKey;
    QString m_serverIpv4AddrIn;
    QString m_serverIpv6AddrIn;
    int m_serverPort = 0;
    bool m_ipv6Enabled = false;
    QStringList m_allowedIPAddressRanges;
  };

  explicit Daemon(QObject* parent);
  ~Daemon();

  static bool parseConfig(const QJsonObject& obj, Config& config);

  virtual bool activate(const Config& config);
  virtual bool deactivate(bool emitSignals = true);

  QString logs();
  void cleanLogs();

 signals:
  void connected();
  void disconnected();

 protected:
  virtual bool run(Op op, const Config& config) = 0;

 protected:
  bool m_connected = false;

  Config m_lastConfig;
};

#endif  // DAEMON_H
