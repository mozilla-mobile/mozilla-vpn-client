/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wireguard.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QList>

#include "daemon/wireguardutils.h"
#include "logger.h"

#if defined(MZ_WASM) || defined(MZ_IOS)
#  include "daemon/obfuscator/dummyobfuscator.h"
#else
#  include "daemon/obfuscator/qprocessobfuscator.h"
#endif

namespace {
Logger logger("WireguardTunnel");
}

// static
std::unique_ptr<Obfuscator> WireGuardTunnel::createObfuscator(
    const InterfaceConfig& config) {
#if defined(MZ_WASM) || defined(MZ_IOS)
  return std::make_unique<DummyObfuscator>(config);
#else
  return std::make_unique<QProcessObfuscator>(config);
#endif
}

bool WireGuardTunnel::maybeStartObfuscator(
    const InterfaceConfig& config, std::unique_ptr<Obfuscator>& obfuscator,
    InterfaceConfig& peerConfig) {
  peerConfig = config;

  // For multi-hop, obfuscate the entry node only.
  if (config.m_obfuscationMethod == Server::ObfuscationMethod::NoObfuscation ||
      config.m_hopType == InterfaceConfig::MultiHopExit) {
    return true;
  }

  obfuscator = createObfuscator(config);
  if (!obfuscator->start()) {
    logger.error() << "Failed to start obfuscator"
                   << config.m_obfuscationMethod;
    return false;
  }

  // Point WireGuard at the relay instead of the real server.
  peerConfig.m_serverIpv4AddrIn = "127.0.0.1";
  // The obfuscator only binds 127.0.0.1, so clear the IPv6 endpoint to keep
  // WireGuard from selecting an [::1]
  peerConfig.m_serverIpv6AddrIn = QString();
  peerConfig.m_serverPort = obfuscator->localPort();
  return true;
}

QJsonObject WireGuardTunnel::getStatus() const {
  Q_ASSERT(wgutils() != nullptr);
  QJsonObject json;
  logger.debug() << "Status request";

  if (!wgutils()->interfaceExists() || m_connections.isEmpty()) {
    json.insert("connected", QJsonValue(false));
    return json;
  }

  const ConnectionState& connection = m_connections.first();
  QList<WireguardUtils::PeerStatus> peers = wgutils()->getPeerStatus();
  for (const WireguardUtils::PeerStatus& status : peers) {
    if (status.m_pubkey != connection.m_config.m_serverPublicKey) {
      continue;
    }
    json.insert("connected", QJsonValue(true));
    json.insert("serverIpv4Gateway",
                QJsonValue(connection.m_config.m_serverIpv4Gateway));
    json.insert("deviceIpv4Address",
                QJsonValue(connection.m_config.m_deviceIpv4Address));
    json.insert("date", connection.m_date.toString());
    json.insert("txBytes", QJsonValue(status.m_txBytes));
    json.insert("rxBytes", QJsonValue(status.m_rxBytes));
    return json;
  }

  json.insert("connected", QJsonValue(false));
  return json;
}

int WireGuardTunnel::checkHandshake() {
  Q_ASSERT(wgutils() != nullptr);

  logger.debug() << "Checking for handshake...";

  int pendingHandshakes = 0;
  QList<WireguardUtils::PeerStatus> peers = wgutils()->getPeerStatus();
  for (ConnectionState& connection : m_connections) {
    const InterfaceConfig& config = connection.m_config;
    if (connection.m_date.isValid()) {
      continue;
    }
    logger.debug() << "awaiting" << logger.keys(config.m_serverPublicKey);

    // Check if the handshake has completed.
    for (const WireguardUtils::PeerStatus& status : peers) {
      if (config.m_serverPublicKey != status.m_pubkey) {
        continue;
      }
      if (status.m_handshake != 0) {
        connection.m_date.setMSecsSinceEpoch(status.m_handshake);
        emit connected(status.m_pubkey);
      }
    }

    if (!connection.m_date.isValid()) {
      pendingHandshakes++;
    }
  }
  return pendingHandshakes;
}

bool WireGuardTunnel::switchServer(const InterfaceConfig& config) {
  Q_ASSERT(m_connections.contains(config.m_hopType));
  const InterfaceConfig& lastConfig =
      m_connections.value(config.m_hopType).m_config;

  // Stand up a new obfuscator for the new endpoint (entry hop only).
  std::unique_ptr<Obfuscator> obfuscator;
  InterfaceConfig peerConfig;
  if (!maybeStartObfuscator(config, obfuscator, peerConfig)) {
    return false;
  }

  // Activate the new peer and its routes.
  if (!wgutils()->updatePeer(peerConfig)) {
    logger.error() << "Server switch failed to update the wireguard interface";
    return false;
  }

  // Take ownership of the new obfuscator (entry hop only). This tears down the
  // previous relay, which the old peer was still using.
  if (config.m_hopType != InterfaceConfig::MultiHopExit) {
    m_obfuscator = std::move(obfuscator);
  }

  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    if (!wgutils()->updateRoutePrefix(ip)) {
      logger.error() << "Server switch failed to update the routing table";
      break;
    }
  }

  // Remove routing entries for the old peer.
  for (const IPAddress& ip : lastConfig.m_allowedIPAddressRanges) {
    if (!config.m_allowedIPAddressRanges.contains(ip)) {
      wgutils()->deleteRoutePrefix(ip);
    }
  }

  // Remove the old peer if it is no longer necessary.
  if (config.m_serverPublicKey != lastConfig.m_serverPublicKey) {
    if (!wgutils()->deletePeer(lastConfig)) {
      return false;
    }
  }

  m_connections[config.m_hopType] = ConnectionState(config);
  return true;
}

bool WireGuardTunnel::deleteInterface() {
  // Cleanup peers and routing
  for (const ConnectionState& state : m_connections) {
    const InterfaceConfig& config = state.m_config;
    logger.debug() << "Deleting routes for" << config.m_hopType;
    for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
      wgutils()->deleteRoutePrefix(ip);
    }
    wgutils()->deletePeer(config);
  }
  m_connections.clear();

  // Delete the obfuscator
  m_obfuscator.reset();

  // Delete the interface
  return wgutils()->deleteInterface();
}

bool WireGuardTunnel::addInterface(const InterfaceConfig& config) {
  return wgutils()->addInterface(config);
}

bool WireGuardTunnel::activate(const InterfaceConfig& config) {
  Q_ASSERT(wgutils() != nullptr);
  // Bring up the wireguard interface if not already done.
  if (!interfaceExists()) {
    // Create the interface.
    if (!addInterface(config)) {
      logger.error() << "Interface creation failed.";
      return false;
    }

    // Bring the interface up.
    if (supportIPUtils()) {
      if (!iputils()->addInterfaceIPs(config)) {
        return false;
      }
      if (!iputils()->setMTUAndUp(config)) {
        return false;
      }
    }

    // Configure LAN exclusion policies
    auto lanAddressRanges = IPAddress::lanAddressRanges();
    if (!wgutils()->excludeLocalNetworks(lanAddressRanges)) {
      logger.error() << "LAN exclusion failed.";
      return false;
    }
  }

  // If an obfuscator is configured, start the relay and rewrite the peer
  // endpoint so WireGuard talks to it instead of the real server.
  std::unique_ptr<Obfuscator> obfuscator;
  InterfaceConfig peerConfig;
  if (!maybeStartObfuscator(config, obfuscator, peerConfig)) {
    return false;
  }

  // Add the peer to this interface.
  if (!wgutils()->updatePeer(peerConfig)) {
    logger.error() << "Peer creation failed.";
    return false;
  }

  // Take ownership of the new obfuscator (entry hop only).
  if (config.m_hopType != InterfaceConfig::MultiHopExit) {
    m_obfuscator = std::move(obfuscator);
  }

  // set routing
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    if (!wgutils()->updateRoutePrefix(ip)) {
      logger.debug() << "Routing configuration failed for"
                     << logger.sensitive(ip.toString());
      return false;
    }
  }
  return true;
}