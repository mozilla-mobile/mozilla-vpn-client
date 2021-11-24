/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnshelper.h"
#include "features/featurecustomdns.h"
#include "features/featurelocalareaaccess.h"
#include "ipaddress.h"
#include "logger.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"
#include "rfc/rfc5735.h"
#include "settingsholder.h"

#include <QHostAddress>

namespace {
Logger logger(LOG_NETWORKING, "DNSHelper");
}

// Returns the DNS Server the user asked for in the Settings;
constexpr const char* MULLVAD_BLOCK_ADS_DNS = "100.64.0.1";
constexpr const char* MULLVAD_BLOCK_TRACKING_DNS = "100.64.0.2";
constexpr const char* MULLVAD_BLOCK_ALL_DNS = "100.64.0.3";

// static
QString DNSHelper::getDNS(const QString& fallback) {
  if (!FeatureCustomDNS::instance()->isSupported()) {
    return fallback;
  }

  auto& settingsHolder = SettingsHolder::instance();

  int dnsProvider = settingsHolder.dnsProvider();
  switch (dnsProvider) {
    case SettingsHolder::Gateway:
      return fallback;
    case SettingsHolder::BlockAll:
      return MULLVAD_BLOCK_ALL_DNS;
    case SettingsHolder::BlockAds:
      return MULLVAD_BLOCK_ADS_DNS;
    case SettingsHolder::BlockTracking:
      return MULLVAD_BLOCK_TRACKING_DNS;
    case SettingsHolder::Custom:
    default:
      break;
  }

  Q_ASSERT(dnsProvider == SettingsHolder::Custom);

  QString dns = settingsHolder.userDNS();
  // User wants to use a Custom DNS, let's check that this is valid.
  if (dns.isEmpty() || !validateUserDNS(dns)) {
    logger.debug()
        << "Saved Custom DNS seems invalid, defaulting to gateway DNS";
    return fallback;
  }

  return dns;
}

// static
bool DNSHelper::isMullvadDNS(const QString& address) {
  IPAddress mullvadAddresses = IPAddress::create("100.64.0.0/24");
  return mullvadAddresses.contains(QHostAddress(address));
}

// static
bool DNSHelper::validateUserDNS(const QString& dns) {
  QHostAddress address = QHostAddress(dns);
  logger.debug() << "checking -> " << dns << "==" << !address.isNull();
  if (address.isNull()) {
    return false;
  }
  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    // We have an issue on Windows with v6 dns, lets disable v6 dns for now.
    // See: https://github.com/mozilla-mobile/mozilla-vpn-client/issues/1714
    return false;
  }
#if defined(MVPN_ANDROID) || defined(MVPN_IOS)
  // Android/IOS rejects loopback as dns
  if (address.isLoopback()) {
    return false;
  }
#endif
  return true;
}

// static
bool DNSHelper::shouldExcludeDNS() {
  auto& settingsHolder = SettingsHolder::instance();
  if (!FeatureCustomDNS::instance()->isSupported()) {
    return false;
  }

  // Only a Custom DNS might require to be routed outside of the VPN-Tunnel
  if (settingsHolder.dnsProvider() != SettingsHolder::DnsProvider::Custom) {
    return false;
  }

  auto dns = settingsHolder.userDNS();
  if (!validateUserDNS(dns)) {
    return false;
  }

  QHostAddress dnsAddress(dns);

  // No need to filter out loopback ip addresses
  if (RFC5735::ipv4LoopbackAddressBlock().contains(dnsAddress) ||
      RFC4291::ipv6LoopbackAddressBlock().contains(dnsAddress)) {
    return false;
  }

  // Edge-case: the DNS is a mullvad one.
  if (isMullvadDNS(dns)) {
    return false;
  }

  bool isLocalDNS =
      RFC1918::contains(dnsAddress) || RFC4193::contains(dnsAddress);
  if (!FeatureLocalAreaAccess::instance()->isSupported() && isLocalDNS) {
    // In case we cant use lan access, we must exclude it (the platform already
    // does the magic for us).
    return false;
  }

  if (isLocalDNS && settingsHolder.localNetworkAccess()) {
    // DNS is lan, but we already excluded local-ip's, all good.
    return false;
  }

  if (!isLocalDNS) {
    return false;
  }
  return true;
}
