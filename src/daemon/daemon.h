/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DAEMON_H
#define DAEMON_H

#include "dnsutils.h"
#include "interfaceconfig.h"
#include "iputils.h"
#include "wireguardutils.h"

#include <QDateTime>

class Daemon : public QObject {
  Q_OBJECT

 public:
  enum Op {
    Up,
    Down,
  };

  explicit Daemon(QObject* parent);
  ~Daemon();

  static Daemon* instance();

  bool parseConfig(const QJsonObject& obj, InterfaceConfig& config);

  virtual QString interfaceName(int hopindex) = 0;
  virtual bool activate(const InterfaceConfig& config);
  virtual bool deactivate(int hopindex, bool emitSignals = true);
  virtual bool deactivateAll(bool emitSignals = true);

  // Explose a JSON object with the daemon status.
  virtual QByteArray getStatus() = 0;

  // Callback before any Activating measure is done
  virtual void prepareActivation(const InterfaceConfig& config){
      Q_UNUSED(config)};

  QString logs();
  void cleanLogs();

 signals:
  void connected(int hopindex);
  void disconnected(int hopindex);
  void backendFailure();

 protected:
  virtual bool run(Op op, const InterfaceConfig& config) {
    Q_UNUSED(op);
    Q_UNUSED(config);
    return true;
  }
  virtual bool supportServerSwitching(const InterfaceConfig& config) const {
    Q_UNUSED(config);
    return false;
  }
  virtual bool switchServer(const InterfaceConfig& config);
  virtual bool supportWGUtils() const { return false; }
  virtual WireguardUtils* wgutils() { return nullptr; }
  virtual bool supportIPUtils() const { return false; }
  virtual IPUtils* iputils() { return nullptr; }
  virtual bool supportDnsUtils() const { return false; }
  virtual DnsUtils* dnsutils() { return nullptr; }

  class ConnectionState {
   public:
    ConnectionState(){};
    ConnectionState(const InterfaceConfig& config) {
      m_config = config;
      m_date = QDateTime::currentDateTime();
    }
    QDateTime m_date;
    InterfaceConfig m_config;
  };
  QMap<int, ConnectionState> m_connections;
};

#endif  // DAEMON_H
