/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnshelper.h"

#include <QHostAddress>

#include "feature.h"
#include "ipaddress.h"
#include "logger.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"
#include "rfc/rfc5735.h"
#include "settingsholder.h"

namespace {
Logger logger("DNSHelper");
}

// Returns the DNS Server the user asked for in the Settings;
constexpr const char* MULLVAD_BLOCK_ADS_DNS = "100.64.0.1";
constexpr const char* MULLVAD_BLOCK_TRACKING_DNS = "100.64.0.2";
constexpr const char* MULLVAD_BLOCK_ALL_DNS = "100.64.0.3";

// static
QString DNSHelper::getDNS(const QString& fallback) {
  if (!Feature::get(Feature::Feature_customDNS)->isSupported()) {
    return fallback;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  int dnsProvider = settingsHolder->dnsProvider();
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

  QString dns = settingsHolder->userDNS();
  // User wants to use a Custom DNS, let's check that this is valid.
  if (dns.isEmpty() || !validateUserDNS(dns)) {
    logger.debug()
        << "Saved Custom DNS seems invalid, defaulting to gateway DNS";
    return fallback;
  }

  return dns;
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
#if defined(MZ_ANDROID) || defined(MZ_IOS)
  // Android/IOS rejects loopback as dns
  if (address.isLoopback()) {
    return false;
  }
#endif

  return true;
}
