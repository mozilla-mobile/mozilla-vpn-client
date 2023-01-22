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

// static
QString DNSHelper::getDNS(const QString& fallback) {
  if (!Feature::get(Feature::Feature_customDNS)->isSupported()) {
    return fallback;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  int dnsProviderFlags = settingsHolder->dnsProviderFlags();

  // Gateway DNS
  if (dnsProviderFlags == SettingsHolder::Gateway) {
    return fallback;
  }

  // Custom DNS
  if (dnsProviderFlags == SettingsHolder::Custom) {
    QString dns = settingsHolder->userDNS();
    // User wants to use a Custom DNS, let's check that this is valid.
    if (dns.isEmpty() || !validateUserDNS(dns)) {
      logger.debug()
          << "Saved Custom DNS seems invalid, defaulting to gateway DNS";
      return fallback;
    }

    return dns;
  }

  static QMap<int, QString> dnsMap{
      {SettingsHolder::BlockAds, "100.64.0.1"},
      {SettingsHolder::BlockTrackers, "100.64.0.2"},
      {SettingsHolder::BlockAds & SettingsHolder::BlockTrackers, "100.64.0.3"},
      {SettingsHolder::BlockMalware, "100.64.0.4"},
      {SettingsHolder::BlockMalware + SettingsHolder::BlockAds, "100.64.0.5"},
      {SettingsHolder::BlockMalware + SettingsHolder::BlockTrackers,
       "100.64.0.6"},
      {SettingsHolder::BlockMalware + SettingsHolder::BlockAds +
           SettingsHolder::BlockTrackers,
       "100.64.0.7"},
  };

  Q_ASSERT(dnsMap.contains(dnsProviderFlags));
  return dnsMap[dnsProviderFlags];
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

// static
void DNSHelper::maybeMigrateDNSProviderFlags() {
  logger.debug() << "Maybe migrate DNS Provider Flags";

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (!settingsHolder->hasDNSProviderDeprecated()) {
    return;
  }

  switch (settingsHolder->dnsProviderDeprecated()) {
    case 0:  // Gateway
      break;

    case 1:  // BlockAll
      // At the time we implement this migration, we just have adblock +
      // tracking
      settingsHolder->setDNSProviderFlags(SettingsHolder::BlockAds &
                                          SettingsHolder::BlockTrackers);
      break;

    case 2:  // BlockAds
      settingsHolder->setDNSProviderFlags(SettingsHolder::BlockAds);
      break;

    case 3:  // BlockTrackers
      settingsHolder->setDNSProviderFlags(SettingsHolder::BlockTrackers);
      break;

    case 4:  // Custom
      settingsHolder->setDNSProviderFlags(SettingsHolder::Custom);
      break;

    default:
      logger.warning() << "Unsupported DNS provider deprecated value"
                       << settingsHolder->dnsProviderDeprecated();
      break;
  }

  settingsHolder->removeDNSProviderDeprecated();
}
