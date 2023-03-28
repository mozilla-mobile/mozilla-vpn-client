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

const QString BLOCK_ADS_DNS = "100.64.0.1";
const QString BLOCK_TRACKERS_DNS = "100.64.0.2";
const QString BLOCK_ADS_TRACKERS_DNS = "100.64.0.3";
const QString BLOCK_MALWARE_DNS = "100.64.0.4";
const QString BLOCK_MALWARE_ADS_DNS = "100.64.0.5";
const QString BLOCK_MALWARE_TRACKERS_DNS = "100.64.0.6";
const QString BLOCK_MALWARE_ADS_TRACKERS_DNS = "100.64.0.7";

// static
QString DNSHelper::getDNS(const QString& fallback) {
  return getDNSDetails(fallback).ipAddress;
}

// static
QString DNSHelper::getDNSType() {
  // `getDNSDetails` takes a string for its fallback DNS server.
  // As `getDNSType` doesn't return a DNS address, it can send
  // any string, including `pretenedDNS`.
  return getDNSDetails("pretendDNS").dnsType;
}

// static
dnsData DNSHelper::getDNSDetails(const QString& fallback) {
  if (!Feature::get(Feature::Feature_customDNS)->isSupported()) {
    return dnsData{fallback, "NoCustomDNSAvailable"};
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  int dnsProviderFlags = settingsHolder->dnsProviderFlags();

  // Gateway DNS
  if (dnsProviderFlags == SettingsHolder::Gateway) {
    return dnsData{fallback, "Default"};
  }

  // Custom DNS
  if (dnsProviderFlags == SettingsHolder::Custom) {
    QString dns = settingsHolder->userDNS();
    // User wants to use a Custom DNS, let's check that this is valid.
    if (dns.isEmpty() || !validateUserDNS(dns)) {
      logger.debug()
          << "Saved Custom DNS seems invalid, defaulting to gateway DNS";
      return dnsData{fallback, "Default"};
    }

    return dnsData{dns, "Custom"};
  }

  static QMap<int, dnsData> dnsMap{
      {SettingsHolder::BlockAds, dnsData{BLOCK_ADS_DNS, "BlockAds"}},
      {SettingsHolder::BlockTrackers,
       dnsData{BLOCK_TRACKERS_DNS, "BlockTrackers"}},
      {SettingsHolder::BlockAds + SettingsHolder::BlockTrackers,
       dnsData{BLOCK_ADS_TRACKERS_DNS, "BlockAdsAndTrackers"}},
      {SettingsHolder::BlockMalware,
       dnsData{BLOCK_MALWARE_DNS, "BlockMalware"}},
      {SettingsHolder::BlockMalware + SettingsHolder::BlockAds,
       dnsData{BLOCK_MALWARE_ADS_DNS, "BlockMalwareAndAds"}},
      {SettingsHolder::BlockMalware + SettingsHolder::BlockTrackers,
       dnsData{BLOCK_MALWARE_TRACKERS_DNS, "BlockMalwareAndTrackers"}},
      {SettingsHolder::BlockMalware + SettingsHolder::BlockAds +
           SettingsHolder::BlockTrackers,
       dnsData{BLOCK_MALWARE_ADS_TRACKERS_DNS,
               "BlockMalwareAndAdsAndTrackers"}},
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
