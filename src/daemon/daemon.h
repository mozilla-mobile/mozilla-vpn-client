/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DAEMON_H
#define DAEMON_H

#include <QDateTime>
#include <QTimer>
#include <memory>

#include "daemonerrors.h"
#include "interfaceconfig.h"
#include "obfuscator/obfuscator.h"
#include "wireguardutils.h"

class DnsUtils;
class IPUtils;
class Obfuscator;
class WireguardUtils;

class Daemon : public QObject {
  Q_OBJECT

 public:
  enum Op {
    Up,
    Down,
    Switch,
  };

  explicit Daemon(QObject* parent);
  ~Daemon();

  virtual bool activate(const InterfaceConfig& config);
  virtual bool deactivate(bool emitSignals);

  virtual QStringList getFeatures() const { return QStringList(); }

  QJsonObject getStatus();
  QString logs();
  QString interfaceName() const;

  Q_INVOKABLE bool activate(const QString& json);
  Q_INVOKABLE bool deactivate() { return deactivate(true); };
  Q_INVOKABLE void cleanLogs();

  virtual bool supportServerSwitching(const InterfaceConfig& config) const;
  virtual bool switchServer(const InterfaceConfig& config);

 signals:
  void connected(const QString& pubkey);
  /**
   * Can be fired if a call to activate() was unsucessfull
   * and connected systems should rollback
   */
  void activationFailure();
  void disconnected();
  void backendFailure(DaemonError reason = DaemonError::ERROR_FATAL);

 private:
  bool maybeUpdateResolvers(const InterfaceConfig& config);
  std::unique_ptr<Obfuscator> createObfuscator(const InterfaceConfig& config);

 protected:
  virtual bool run(Op op, const InterfaceConfig& config) {
    Q_UNUSED(op);
    Q_UNUSED(config);
    return true;
  }
  virtual WireguardUtils* wgutils() const = 0;
  virtual IPUtils* iputils() { return nullptr; }
  virtual DnsUtils* dnsutils() { return nullptr; }

  void abortBackendFailure();
  void checkHandshake();

  QHostAddress chooseAddress(const QStringList& candidates);
  bool activateMultihop(const InterfaceConfig& config);
  void finishMultihop();
  bool activateObfuscator(const InterfaceConfig& config,
                           WireguardUtils::PeerConfig &peer);

  QMap<QString, WireguardUtils::PeerConfig> m_connections;
  QStringList m_awaiting;
  InterfaceConfig m_config;
  QTimer m_handshakeTimer;
  std::unique_ptr<Obfuscator> m_obfuscator;

  friend class TestDaemon;
};

#endif  // DAEMON_H
