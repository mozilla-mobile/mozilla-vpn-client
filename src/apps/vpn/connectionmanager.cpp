/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionmanager.h"

//#include "controller.h"
#include "app.h"
#include "appconstants.h"
#include "apppermission.h"
#include "captiveportal/captiveportal.h"
#include "controllerimpl.h"
#include "dnshelper.h"
#include "feature.h"
#include "frontend/navigator.h"
#include "ipaddress.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/devicemodel.h"
#include "models/keys.h"
#include "models/server.h"
#include "models/servercountrymodel.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "rfc/rfc1112.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"
#include "serveri18n.h"
#include "serverlatency.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "tasks/function/taskfunction.h"
#include "tasks/heartbeat/taskheartbeat.h"
#include "taskscheduler.h"
#include "tutorial/tutorial.h"

//constexpr const uint32_t TIMER_MSEC = 1000;

// X connection retries.
constexpr const int CONNECTION_MAX_RETRY = 9;

//constexpr const uint32_t CONFIRMING_TIMOUT_SEC = 10;
//constexpr const uint32_t HANDSHAKE_TIMEOUT_SEC = 15;

#ifndef MZ_IOS
// The Mullvad proxy services are located at internal IPv4 addresses in the
// 10.124.0.0/20 address range, which is a subset of the 10.0.0.0/8 Class-A
// private address range.
constexpr const char* MULLVAD_PROXY_RANGE = "10.124.0.0";
constexpr const int MULLVAD_PROXY_RANGE_LENGTH = 20;
#endif

namespace {
Logger logger("Connection Manager");
}

ConnectionManager::ConnectionManager() {
  MZ_COUNT_CTOR(ConnectionManager);
  
  m_connectingTimer.setSingleShot(true);
  m_handshakeTimer.setSingleShot(true);

  connect(&m_timer, &QTimer::timeout, this, &ConnectionManager::timerTimeout);

  connect(&m_connectingTimer, &QTimer::timeout, this, [this]() {
    m_enableDisconnectInConfirming = true;
    emit enableDisconnectInConfirmingChanged();
  });

  connect(&m_handshakeTimer, &QTimer::timeout, this,
          &ConnectionManager::handshakeTimeout);

  LogHandler::instance()->registerLogSerializer(this);
}

ConnectionManager::~ConnectionManager() {
  MZ_COUNT_DTOR(ConnectionManager);
}

void ConnectionManager::initialize() {
  logger.debug() << "Initializing the connection manager";
}

qint64 ConnectionManager::time() const {
  if (m_connectedTimeInUTC.isValid()) {
    return m_connectedTimeInUTC.secsTo(QDateTime::currentDateTimeUtc());
  }
  return 0;
}

void ConnectionManager::timerTimeout() {
  Q_ASSERT(m_state == StateOn);
  emit timeChanged();
}

void ConnectionManager::handshakeTimeout() {
  logger.debug() << "Timeout while waiting for handshake";

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(!m_activationQueue.isEmpty());

  // Block the offending server and try again.
  InterfaceConfig& hop = m_activationQueue.first();
  vpn->serverLatency()->setCooldown(
      hop.m_serverPublicKey, AppConstants::SERVER_UNRESPONSIVE_COOLDOWN_SEC);

  emit handshakeFailed(hop.m_serverPublicKey);

  if (m_nextStep != None) {
//    deactivate();
    return;
  }

  // Try again, again if there are sufficient retries left.
  ++m_connectionRetry;
  emit connectionRetryChanged();
  logger.info() << "Connection attempt " << m_connectionRetry;
  if (m_connectionRetry == 1) {
    logger.info() << "Connection Attempt: Using Port 53 Option this time.";
    // On the first retry, opportunisticly try again using the port 53
    // option enabled, if that feature is disabled.
    activateInternal(ForceDNSPort, RandomizeServerSelection);
    return;
  } else if (m_connectionRetry < CONNECTION_MAX_RETRY) {
    activateInternal(DoNotForceDNSPort, RandomizeServerSelection);
    return;
  }

  // Otherwise, the give up and report the location as unavailable.
  logger.error() << "Connection retries exhausted, giving up";
  serverUnavailable();
}

void ConnectionManager::serverUnavailable() {
  logger.error() << "server unavailable";

  m_nextStep = ServerUnavailable;

  if (m_state == StateOn || m_state == StateSwitching ||
      m_state == StateSilentSwitching || m_state == StateConnecting ||
      m_state == StateConfirming || m_state == StateCheckSubscription) {
//    deactivate();
    return;
  }
}
void ConnectionManager::activateInternal(DNSPortPolicy dnsPort,
                                  ServerSelectionPolicy serverSelectionPolicy) {
  logger.debug() << "Activation internal";
  Q_ASSERT(m_impl);

  clearConnectedTime();
  m_handshakeTimer.stop();
  m_activationQueue.clear();

  Server exitServer =
      serverSelectionPolicy == DoNotRandomizeServerSelection &&
              !m_serverData.exitServerPublicKey().isEmpty()
          ? MozillaVPN::instance()->serverCountryModel()->server(
                m_serverData.exitServerPublicKey())
          : Server::weightChooser(m_serverData.exitServers());
  if (!exitServer.initialized()) {
    logger.error() << "Empty exit server list in state" << m_state;
    serverUnavailable();
    return;
  }

  MozillaVPN* vpn = MozillaVPN::instance();
  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());
  SettingsHolder* settingsHolder = SettingsHolder::instance();

  // Prepare the exit server's connection data.
  InterfaceConfig exitConfig;
  exitConfig.m_privateKey = vpn->keys()->privateKey();
  exitConfig.m_deviceIpv4Address = device->ipv4Address();
  exitConfig.m_deviceIpv6Address = device->ipv6Address();
  exitConfig.m_serverIpv4Gateway = exitServer.ipv4Gateway();
  exitConfig.m_serverIpv6Gateway = exitServer.ipv6Gateway();
  exitConfig.m_serverPublicKey = exitServer.publicKey();
  exitConfig.m_serverIpv4AddrIn = exitServer.ipv4AddrIn();
  exitConfig.m_serverIpv6AddrIn = exitServer.ipv6AddrIn();
  exitConfig.m_serverPort = exitServer.choosePort();
  exitConfig.m_allowedIPAddressRanges = getAllowedIPAddressRanges(exitServer);
  exitConfig.m_excludedAddresses = getExcludedAddresses();
  exitConfig.m_dnsServer = DNSHelper::getDNS(exitServer.ipv4Gateway());
#if defined(MZ_ANDROID) || defined(MZ_IOS)
  exitConfig.m_installationId = settingsHolder->installationId();
#endif
  logger.debug() << "DNS Set" << exitConfig.m_dnsServer;

  // Splittunnel-feature could have been disabled due to a driver conflict.
  if (Feature::get(Feature::Feature_splitTunnel)->isSupported()) {
    exitConfig.m_vpnDisabledApps = settingsHolder->vpnDisabledApps();
  }
  if (Feature::get(Feature::Feature_alwaysPort53)->isSupported()) {
    dnsPort = ForceDNSPort;
  }

  // For single-hop connections, exclude the entry server
  if (!Feature::get(Feature::Feature_multiHop)->isSupported() ||
      !m_serverData.multihop()) {
    logger.info() << "Activating single hop";
    exitConfig.m_hopType = InterfaceConfig::SingleHop;

    // If requested, force the use of port 53/DNS.
    if (dnsPort == ForceDNSPort) {
      logger.info() << "Forcing port 53";
      exitConfig.m_serverPort = 53;
    }
  }
  // For controllers that support multiple hops, create a queue of connections.
  // The entry server should start first, followed by the exit server.
  else if (m_impl->multihopSupported()) {
    logger.info() << "Activating multi-hop (through platform controller)";
    exitConfig.m_hopType = InterfaceConfig::MultiHopExit;

    Server entryServer =
        serverSelectionPolicy == DoNotRandomizeServerSelection &&
                !m_serverData.entryServerPublicKey().isEmpty()
            ? MozillaVPN::instance()->serverCountryModel()->server(
                  m_serverData.entryServerPublicKey())
            : Server::weightChooser(m_serverData.entryServers());

    if (!entryServer.initialized()) {
      logger.error() << "Empty entry server list in state" << m_state;
      serverUnavailable();
      return;
    }

    InterfaceConfig entryConfig;
    entryConfig.m_privateKey = vpn->keys()->privateKey();
    entryConfig.m_deviceIpv4Address = device->ipv4Address();
    entryConfig.m_deviceIpv6Address = device->ipv6Address();
    entryConfig.m_serverPublicKey = entryServer.publicKey();
    entryConfig.m_serverIpv4AddrIn = entryServer.ipv4AddrIn();
    entryConfig.m_serverIpv6AddrIn = entryServer.ipv6AddrIn();
    entryConfig.m_serverPort = entryServer.choosePort();
    entryConfig.m_hopType = InterfaceConfig::MultiHopEntry;
    entryConfig.m_allowedIPAddressRanges.append(
        IPAddress(exitServer.ipv4AddrIn()));
    entryConfig.m_allowedIPAddressRanges.append(
        IPAddress(exitServer.ipv6AddrIn()));

    // If requested, force the use of port 53/DNS.
    if (dnsPort == ForceDNSPort) {
      logger.info() << "Forcing port 53";
      entryConfig.m_serverPort = 53;
    }

    m_activationQueue.append(entryConfig);
  }
  // Otherwise, we can approximate multihop support by redirecting the
  // connection to the exit server via the multihop port.
  else {
    logger.info() << "Activating multi-hop (through multihop port)";
    exitConfig.m_hopType = InterfaceConfig::SingleHop;

    Server entryServer =
        serverSelectionPolicy == DoNotRandomizeServerSelection &&
                !m_serverData.entryServerPublicKey().isEmpty()
            ? MozillaVPN::instance()->serverCountryModel()->server(
                  m_serverData.entryServerPublicKey())
            : Server::weightChooser(m_serverData.entryServers());

    if (!entryServer.initialized()) {
      logger.error() << "Empty entry server list in state" << m_state;
      serverUnavailable();
      return;
    }

    // NOTE: For platforms without multihop support, we cannot emulate multihop
    // and use port 53 at the same time. If the user has selected both options
    // then let's choose multihop.
    exitConfig.m_serverPort = exitServer.multihopPort();
    exitConfig.m_serverIpv4AddrIn = entryServer.ipv4AddrIn();
    exitConfig.m_serverIpv6AddrIn = entryServer.ipv6AddrIn();
  }

  m_activationQueue.append(exitConfig);
  m_serverData.setEntryServerPublicKey(
      m_activationQueue.first().m_serverPublicKey);
  m_serverData.setExitServerPublicKey(
      m_activationQueue.last().m_serverPublicKey);

  m_ping_received = false;
  m_ping_canary.start(m_activationQueue.first().m_serverIpv4AddrIn,
                      "0.0.0.0/0");
  logger.info() << "Canary Ping Started";
  activateNext();
}

void ConnectionManager::clearConnectedTime() {
  m_connectedTimeInUTC = QDateTime();
  emit timeChanged();
  m_timer.stop();
}

QList<IPAddress> ConnectionManager::getAllowedIPAddressRanges(
    const Server& exitServer) {
  logger.debug() << "Computing the allowed IP addresses";

  QList<IPAddress> list;

#ifdef MZ_IOS
  // Note: On iOS, we use the `excludeLocalNetworks` flag to ensure
  // LAN traffic is allowed through. This is in the swift code.

  Q_UNUSED(exitServer);

  logger.debug() << "Catch all IPv4";
  list.append(IPAddress("0.0.0.0/0"));

  logger.debug() << "Catch all IPv6";
  list.append(IPAddress("::0/0"));
#else
  QList<IPAddress> excludeIPv4s;
  QList<IPAddress> excludeIPv6s;
  // For multi-hop connections, the last entry in the server list is the
  // ingress node to the network of wireguard servers, and must not be
  // routed through the VPN.

  // filtering out the RFC1918 local area network
  logger.debug() << "Filtering out the local area networks (rfc 1918)";
  excludeIPv4s.append(RFC1918::ipv4());

  logger.debug() << "Filtering out the local area networks (rfc 4193)";
  excludeIPv6s.append(RFC4193::ipv6());

  logger.debug() << "Filtering out multicast addresses";
  excludeIPv4s.append(RFC1112::ipv4MulticastAddressBlock());
  excludeIPv6s.append(RFC4291::ipv6MulticastAddressBlock());

  logger.debug() << "Filtering out explicitely-set network address ranges";
  for (const QString& ipv4String :
       SettingsHolder::instance()->excludedIpv4Addresses()) {
    excludeIPv4s.append(IPAddress(ipv4String));
  }
  for (const QString& ipv6String :
       SettingsHolder::instance()->excludedIpv6Addresses()) {
    excludeIPv6s.append(IPAddress(ipv6String));
  }

  // Allow access to the internal gateway addresses.
  logger.debug() << "Allow the IPv4 gateway:" << exitServer.ipv4Gateway();
  list.append(IPAddress(QHostAddress(exitServer.ipv4Gateway()), 32));
  logger.debug() << "Allow the IPv6 gateway:" << exitServer.ipv6Gateway();
  list.append(IPAddress(QHostAddress(exitServer.ipv6Gateway()), 128));

  // Ensure that the Mullvad proxy services are always allowed.
  list.append(
      IPAddress(QHostAddress(MULLVAD_PROXY_RANGE), MULLVAD_PROXY_RANGE_LENGTH));

  // Allow access to everything not covered by an excluded address.
  QList<IPAddress> allowedIPv4 = {IPAddress("0.0.0.0/0")};
  list.append(IPAddress::excludeAddresses(allowedIPv4, excludeIPv4s));
  QList<IPAddress> allowedIPv6 = {IPAddress("::/0")};
  list.append(IPAddress::excludeAddresses(allowedIPv6, excludeIPv6s));
#endif

  return list;
}

QStringList ConnectionManager::getExcludedAddresses() {
  logger.debug() << "Computing the excluded IP addresses";

  QStringList list;

  // filtering out the captive portal endpoint
  if (Feature::get(Feature::Feature_captivePortal)->isSupported() &&
      SettingsHolder::instance()->captivePortalAlert()) {
    CaptivePortal* captivePortal = MozillaVPN::instance()->captivePortal();

    for (const QString& address : captivePortal->ipv4Addresses()) {
      logger.debug() << "Filtering out the captive portal address:" << address;
      list.append(address);
    }
    for (const QString& address : captivePortal->ipv6Addresses()) {
      logger.debug() << "Filtering out the captive portal address:" << address;
      list.append(address);
    }
  }

  return list;
}

void ConnectionManager::activateNext() {
//  MozillaVPN* vpn = MozillaVPN::instance();
//  const Device* device = vpn->deviceModel()->currentDevice(vpn->keys());
//  if (device == nullptr) {
//    REPORTERROR(ErrorHandler::AuthenticationError, "controller");
//    vpn->reset(false);
//    return;
//  }
//  const InterfaceConfig& config = m_activationQueue.first();
//
//  logger.debug() << "Activating peer" << logger.keys(config.m_serverPublicKey);
//  m_handshakeTimer.start(HANDSHAKE_TIMEOUT_SEC * 1000);
//  m_impl->activate(config, stateToReason(m_state));
//
//  // Move to the confirming state if we are awaiting any connection handshakes.
//  setState(StateConfirming);
}
