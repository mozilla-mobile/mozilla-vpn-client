/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDateTime>
#include <QJsonObject>

#include "interfaceconfig.h"
#include "models/server.h"

class Tunnel : public QObject {
  Q_OBJECT
 public:
  enum Op {
    Up,
    Down,
    Switch,
  };
  explicit Tunnel(QObject* parent) : QObject(parent){};

  virtual bool activate(const InterfaceConfig& config) = 0;
  virtual bool switchServer(const InterfaceConfig& config) = 0;
  virtual int checkHandshake() = 0;
  virtual QJsonObject getStatus() const = 0;
  virtual Server::ProtocolType protocolType() const = 0;
  virtual bool interfaceExists() = 0;
  virtual QString interfaceName() const = 0;
  virtual bool addInterface(const InterfaceConfig& config) = 0;
  virtual bool deleteInterface() = 0;
  virtual void excludeApp(const QString& app) = 0;
  virtual void resetApp(const QString& app) = 0;
  virtual void resetAllApps() = 0;
  virtual bool supportSplitTunnel() = 0;

  bool removeInterfaceIfExists() {
    if (interfaceExists()) {
      // logger.warning() << "Device already exists. Let's remove it.";
      if (!deleteInterface()) {
        // logger.error() << "Failed to remove the device.";
        return false;
      }
    }
    return true;
  }

  class ConnectionState {
   public:
    ConnectionState(){};
    ConnectionState(const InterfaceConfig& config) { m_config = config; }
    QDateTime m_date;
    InterfaceConfig m_config;
  };
  QMap<InterfaceConfig::HopType, ConnectionState> m_connections;
  virtual bool run(Op op, const InterfaceConfig& config) {
    if (op == Op::Switch || op == Op::Up) {
      m_connections[config.m_hopType] = ConnectionState(config);
      if (op == Op::Up) {
        return activate(config);
      }
    }
    if (op == Op::Down) {
      m_connections.remove(config.m_hopType);
    }
    return true;
  }
 signals:
  void backendFailure();
  void connected(const QString& pubkey);
  void disconnected();
};

#endif  // PROTOCOL_H