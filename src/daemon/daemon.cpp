/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemon.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaEnum>
#include <QTimer>

#include "dnsutils.h"
#include "iputils.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "obfuscator/obfuscator.h"
#include "wireguardutils.h"

#if defined(MZ_WASM) || defined(MZ_IOS)
#  include "obfuscator/dummyobfuscator.h"
#else
#  include "obfuscator/qprocessobfuscator.h"
#endif

constexpr int HANDSHAKE_POLL_MSEC = 250;

namespace {
Logger logger("Daemon");
}  // namespace

Daemon::Daemon(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(Daemon);

  logger.debug() << "Daemon created";

  m_handshakeTimer.setSingleShot(true);
  connect(&m_handshakeTimer, &QTimer::timeout, this, &Daemon::checkHandshake);
}

Daemon::~Daemon() {
  MZ_COUNT_DTOR(Daemon);
  logger.debug() << "Daemon released";
}

QString Daemon::interfaceName() const { return wgutils()->interfaceName(); }

bool Daemon::activate(const QString& json) {
  QJsonDocument jsDocument = QJsonDocument::fromJson(json.toUtf8());
  InterfaceConfig ifConfig(jsDocument.object());

  if (!ifConfig.isValid()) {
    return false;
  }

  return activate(ifConfig);
}

bool Daemon::activate(const InterfaceConfig& config) {
  Q_ASSERT(wgutils() != nullptr);

  // There are 3 possible scenarios in which this method is called:
  //
  // 1. the VPN is off: the method tries to enable the VPN.
  // 2. the VPN is on and the platform doesn't support the server-switching:
  //    this method calls deactivate() and then it continues as 1.
  // 3. the VPN is on and the platform supports the server-switching: this
  //    method calls switchServer().
  //
  // At the end, if the activation succeds, the `connected` signal is emitted.
  // If the activation abort's for any reason `the `activationFailure` signal is
  // emitted.
  logger.debug() << "Activating interface.";
  auto emit_failure_guard = qScopeGuard([this] { emit activationFailure(); });

  // Only supporting single-hop for now - will fix in a followup commit.
  WireguardUtils::PeerConfig peer(config.m_serverPublicKey);
  peer.m_port = config.m_serverPort;
  peer.m_allowedips = config.m_allowedIPAddressRanges;
  peer.m_multihop = !config.m_multihopPublicKey.isEmpty();

  QStringList serverAddrs;
  serverAddrs.append(config.m_serverIpv4AddrIn);
  serverAddrs.append(config.m_serverIpv6AddrIn);
  peer.m_address = chooseAddress(serverAddrs);
  if (peer.m_address.isNull()) {
    logger.debug() << "Server is unreachable via any route";
    return false;
  }

  if (!m_connections.isEmpty()) {
    if (supportServerSwitching(config)) {
      logger.debug() << "Already connected. Server switching supported.";

      if (!switchServer(config)) {
        return false;
      }

#if defined(MZ_MACOS)
      // macOS restores the resolvers by terminating the DNS manager process,
      // so they must be reverted before a new configuration can be applied
      // (see https://mozilla-hub.atlassian.net/browse/VPN-7752)
      if (!dnsutils()->restoreResolvers()) {
        return false;
      }
#endif

      // Elsewhere updateResolvers() replaces the configuration in place.
      // Reverting first would expose the physical link's DNS until it lands
      if (!maybeUpdateResolvers(config)) {
        return false;
      }

      bool status = run(Switch, config);
      logger.debug() << "Connection status:" << status;
      if (status) {
        m_connections[peer.m_pubkey] = peer;
        m_awaiting.append(peer.m_pubkey);
        m_handshakeTimer.start(HANDSHAKE_POLL_MSEC);
        m_config = config;
        emit_failure_guard.dismiss();
        return true;
      }
      return false;
    }

    logger.warning() << "Already connected. Server switching not supported.";
    if (!deactivate(false)) {
      return false;
    }

    Q_ASSERT(m_connections.isEmpty());
    if (activate(config)) {
      emit_failure_guard.dismiss();
      return true;
    }
    return false;
  }

  // Bring up the wireguard interface if not already done.
  if (!wgutils()->interfaceExists()) {
    // Create the interface.
    if (!wgutils()->addInterface(config)) {
      logger.error() << "Interface creation failed.";
      return false;
    }

    // Bring the interface up.
    IPUtils* ipu = iputils();
    if (ipu) {
      if (!ipu->addInterfaceIPs(config)) {
        return false;
      }
      if (!ipu->setMTUAndUp(config)) {
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

  // Apply obfuscation to the peer.
  if (!activateObfuscator(config, peer)) {
    logger.error() << "Failed to start obfuscator"
                   << config.m_obfuscationMethod;
    return false;
  }

  // Add the peer to this interface.
  if (!wgutils()->updatePeer(peer)) {
    logger.error() << "Peer creation failed.";
    return false;
  }

  if (!maybeUpdateResolvers(config)) {
    return false;
  }

  // set routing
  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    if (!wgutils()->updateRoutePrefix(ip)) {
      logger.debug() << "Routing configuration failed for"
                     << logger.sensitive(ip.toString());
      return false;
    }
  }

  bool status = run(Up, config);
  logger.debug() << "Connection status:" << status;
  if (status) {
    m_connections[peer.m_pubkey] = peer;
    m_awaiting.append(peer.m_pubkey);
    m_handshakeTimer.start(HANDSHAKE_POLL_MSEC);
    m_config = config;
    emit_failure_guard.dismiss();
    return true;
  }
  return false;
}

// Select an address to reach the wireguard server from a list of candidates.
QHostAddress Daemon::chooseAddress(const QStringList& candidates) {
  for (const QString& i : candidates) {
    if (i.isEmpty()) {
      continue;
    }

    QHostAddress addr(i);
    if (addr.isNull() || !wgutils()->isRouteReachable(addr)) {
      continue;
    }
    return addr;
  }

  return QHostAddress();
}

// Activate the obfuscator for a given peer.
bool Daemon::activateObfuscator(const InterfaceConfig& config,
                                WireguardUtils::PeerConfig &peer) {
  if (config.m_obfuscationMethod == Server::ObfuscationMethod::NoObfuscation) {
    return true;
  }                               

  // If an obfuscator is configured, start the relay and rewrite the peer
  // endpoint so WireGuard talks to it instead of the real server.
  // For multi-hop configure obfuscator only on the entry node.
  std::unique_ptr<Obfuscator> obfuscator = createObfuscator(config);

#if defined(MZ_WINDOWS)
  // Add exclusion route for exit server to prevent loopbacks
  QList<IPAddress> obfuscatorServer(IPAddress(peer.m_address));
  wgutils()->excludeLocalNetworks(obfuscatorServer);
#endif

  if (!obfuscator->start()) {
    logger.error() << "Failed to start obfuscator"
                   << config.m_obfuscationMethod;
    return false;
  }

  // Take ownership of the new obfuscator.
  m_obfuscator = std::move(obfuscator);

  peer.m_address = QHostAddress(QHostAddress::LocalHost);
  peer.m_port = obfuscator->localPort();
  return true;
}

bool Daemon::maybeUpdateResolvers(const InterfaceConfig& config) {
  QList<QHostAddress> resolvers;
  resolvers.append(QHostAddress(config.m_dnsServer));

  // If the DNS is not the Gateway, it's a user defined DNS
  // thus, not add any other :)
  if (config.m_dnsServer == config.m_serverIpv4Gateway) {
    resolvers.append(QHostAddress(config.m_serverIpv6Gateway));
  }

  if (!dnsutils()->updateResolvers(wgutils()->interfaceName(), resolvers)) {
    return false;
  }

  return true;
}

bool Daemon::deactivate(bool emitSignals) {
  Q_ASSERT(wgutils() != nullptr);

  // Deactivate the main interface.
  if (!m_connections.isEmpty()) {
    if (!run(Down, m_config)) {
      return false;
    }
  }

  // Emit signals upon return.
  auto guard = qScopeGuard([&]() {
    if (emitSignals) {
      emit disconnected();
    }
  });

  // Cleanup DNS
  if (!dnsutils()->restoreResolvers()) {
    logger.warning() << "Failed to restore DNS resolvers.";
  }

  // Cleanup peers and routing
  for (const WireguardUtils::PeerConfig& peer : m_connections) {
    logger.debug() << "Deleting routes for" << peer.m_pubkey;
    for (const IPAddress& ip : peer.m_allowedips) {
      wgutils()->deleteRoutePrefix(ip);
    }
    wgutils()->deletePeer(peer);
  }
  m_awaiting.clear();
  m_connections.clear();

  // Delete the obfuscator
  m_obfuscator.reset();

  // Delete the interface
  return wgutils()->deleteInterface();
}

QString Daemon::logs() {
  QString output;

  {
    QTextStream out(&output);
    LogHandler::instance()->writeLogs(out);
  }

  return output;
}

void Daemon::cleanLogs() { LogHandler::instance()->cleanupLogs(); }

std::unique_ptr<Obfuscator> Daemon::createObfuscator(
    const InterfaceConfig& config) {
#if defined(MZ_WASM) || defined(MZ_IOS)
  return std::make_unique<DummyObfuscator>(config);
#else
  return std::make_unique<QProcessObfuscator>(config);
#endif
}

bool Daemon::supportServerSwitching(const InterfaceConfig& config) const {
  if (!m_config.isValid()) {
    return false;
  }

  // Gateways are intentionally not compared: a different-gateway switch is
  // still handled in-place by switchServer(), avoiding a teardown/leak gap.
  return m_config.m_privateKey == config.m_privateKey &&
         m_config.m_deviceIpv4Address == config.m_deviceIpv4Address &&
         m_config.m_deviceIpv6Address == config.m_deviceIpv6Address;
}

bool Daemon::switchServer(const InterfaceConfig& config) {
  Q_ASSERT(wgutils() != nullptr);

  // Only supporting single-hop for now - will fix in a followup commit.
  logger.debug() << "Switching server";
  Q_ASSERT(!m_connections.isEmpty());
  const WireguardUtils::PeerConfig& oldPeer = m_connections.first();

  WireguardUtils::PeerConfig peer(config.m_serverPublicKey);
  peer.m_port = config.m_serverPort;
  peer.m_allowedips = config.m_allowedIPAddressRanges;
  peer.m_multihop = !config.m_multihopPublicKey.isEmpty();

  QStringList serverAddrs;
  serverAddrs.append(config.m_serverIpv4AddrIn);
  serverAddrs.append(config.m_serverIpv6AddrIn);
  peer.m_address = chooseAddress(serverAddrs);
  if (peer.m_address.isNull()) {
    logger.debug() << "Server is unreachable via any route";
    return false;
  }

  if (!activateObfuscator(config, peer)) {
    logger.error() << "Failed to start obfuscator on switch"
                   << config.m_obfuscationMethod;
    return false;
  }

  // Activate the new peer and its routes.
  if (!wgutils()->updatePeer(peer)) {
    logger.error()
        << "Server switch failed to update the peer wireguard config";
    return false;
  }

  for (const IPAddress& ip : config.m_allowedIPAddressRanges) {
    if (!wgutils()->updateRoutePrefix(ip)) {
      logger.error() << "Server switch failed to update the routing table";
      break;
    }
  }

  // Remove routing entries for the old peer.
  for (const IPAddress& ip : oldPeer.m_allowedips) {
    if (!config.m_allowedIPAddressRanges.contains(ip)) {
      wgutils()->deleteRoutePrefix(ip);
    }
  }

  // Remove the old peer if it is no longer necessary.
  if (config.m_serverPublicKey != oldPeer.m_pubkey) {
    if (!wgutils()->deletePeer(oldPeer)) {
      return false;
    }
    m_connections.remove(oldPeer.m_pubkey);
  }

  m_connections[peer.m_pubkey] = peer;
  m_awaiting.append(peer.m_pubkey);
  return true;
}

QJsonObject Daemon::getStatus() {
  Q_ASSERT(wgutils() != nullptr);
  QJsonObject json;
  logger.debug() << "Status request";

  json.insert("version", QCoreApplication::applicationVersion());
  QJsonArray features;
  for (const QString& f : getFeatures()) {
    features.append(f);
  }
  json.insert("features", features);

  if (!wgutils()->interfaceExists() || m_connections.isEmpty()) {
    json.insert("connected", QJsonValue(false));
    return json;
  }

  QList<WireguardUtils::PeerStatus> peers = wgutils()->getPeerStatus();
  for (const WireguardUtils::PeerStatus& status : peers) {
    if (status.m_pubkey != m_config.m_serverPublicKey) {
      continue;
    }

    json.insert("connected", QJsonValue(true));
    json.insert("serverIpv4Gateway", QJsonValue(m_config.m_serverIpv4Gateway));
    json.insert("deviceIpv4Address", QJsonValue(m_config.m_deviceIpv4Address));
    if (status.m_handshake != 0) {
      QDateTime date = QDateTime::fromMSecsSinceEpoch(status.m_handshake);
      json.insert("date", date.toString());
    }
    json.insert("txBytes", QJsonValue(status.m_txBytes));
    json.insert("rxBytes", QJsonValue(status.m_rxBytes));
    return json;
  }

  json.insert("connected", QJsonValue(false));
  return json;
}

void Daemon::checkHandshake() {
  Q_ASSERT(wgutils() != nullptr);

  logger.debug() << "Checking for handshake...";

  QList<WireguardUtils::PeerStatus> peers = wgutils()->getPeerStatus();
  QStringList unfinished;
  for (const QString& pubkey : m_awaiting) {
    Q_ASSERT(m_connections.contains(pubkey));
    logger.debug() << "awaiting" << logger.keys(pubkey);

    // Check if the handshake has completed.
    for (const WireguardUtils::PeerStatus& status : peers) {
      if (status.m_pubkey != pubkey) {
        continue;
      }
      if (status.m_handshake == 0) {
        unfinished.append(pubkey);
        continue;
      }
      
      emit connected(status.m_pubkey);
    }
  }

  // Check again if there were connections that haven't completed a handshake.
  m_awaiting.swap(unfinished);
  if (!m_awaiting.isEmpty()) {
    m_handshakeTimer.start(HANDSHAKE_POLL_MSEC);
  }
}

void Daemon::abortBackendFailure() {
  logger.warning() << "Backend failure occured - disconnecting";
  emit backendFailure();
  deactivate();
}
