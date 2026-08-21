/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARD_H
#define WIREGUARD_H

#include <memory>

#include "daemon/iputils.h"
#include "daemon/obfuscator/obfuscator.h"
#include "daemon/wireguardutils.h"
#include "tunnel.h"

class WireGuardTunnel : public Tunnel {
  Q_OBJECT
 public:
  WireGuardTunnel(QObject* parent) : Tunnel(parent) {};
  Server::ProtocolType protocolType() const override {
    return Server::ProtocolType::WireGuard;
  }
  QString interfaceName() const override { return wgutils()->interfaceName(); };
  bool interfaceExists() override { return wgutils()->interfaceExists(); };
  QJsonObject getStatus() const override;
  int checkHandshake() override;
  bool addInterface(const InterfaceConfig& config) override;
  bool deleteInterface() override;
  bool switchServer(const InterfaceConfig& config) override;
  bool activate(const InterfaceConfig& config) override;
  virtual bool supportIPUtils() const { return false; }

 private:
  virtual WireguardUtils* wgutils() const = 0;
  virtual IPUtils* iputils() { return nullptr; }

  static std::unique_ptr<Obfuscator> createObfuscator(
      const InterfaceConfig& config);

  // If `config` asks for obfuscation, start a relay and return a copy of the
  // config whose peer endpoint points at it. `obfuscator` receives the running
  // relay, which the caller must keep alive for as long as the peer is up.
  // Returns false only if the relay was requested but failed to start.
  bool maybeStartObfuscator(const InterfaceConfig& config,
                            std::unique_ptr<Obfuscator>& obfuscator,
                            InterfaceConfig& peerConfig);

  std::unique_ptr<Obfuscator> m_obfuscator;
};

#endif  // WIREGUARD_H
