/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsholder.h"
#include "cryptosettings.h"
#include "leakdetector.h"
#include "logger.h"

#include <QSettings>

constexpr bool SETTINGS_IPV6ENABLED_DEFAULT = true;
constexpr bool SETTINGS_LOCALNETWORKACCESS_DEFAULT = false;
constexpr bool SETTINGS_UNSECUREDNETWORKALERT_DEFAULT = false;
constexpr bool SETTINGS_CAPTIVEPORTALALERT_DEFAULT = false;
constexpr bool SETTINGS_STARTATBOOT_DEFAULT = false;
constexpr bool SETTINGS_PROTECTSELECTEDAPPS_DEFAULT = true;

constexpr const char* SETTINGS_IPV6ENABLED = "ipv6Enabled";
constexpr const char* SETTINGS_LOCALNETWORKACCESS = "localNetworkAccess";
constexpr const char* SETTINGS_UNSECUREDNETWORKALERT = "unsecuredNetworkAlert";
constexpr const char* SETTINGS_CAPTIVEPORTALALERT = "captivePortalAlert";
constexpr const char* SETTINGS_STARTATBOOT = "startAtBoot";
constexpr const char* SETTINGS_LANGUAGECODE = "languageCode";
constexpr const char* SETTINGS_TOKEN = "token";
constexpr const char* SETTINGS_SERVERS = "servers";
constexpr const char* SETTINGS_PRIVATEKEY = "privateKey";
constexpr const char* SETTINGS_USER_AVATAR = "user/avatar";
constexpr const char* SETTINGS_USER_DISPLAYNAME = "user/displayName";
constexpr const char* SETTINGS_USER_EMAIL = "user/email";
constexpr const char* SETTINGS_USER_MAXDEVICES = "user/maxDevices";
constexpr const char* SETTINGS_USER_SUBSCRIPTIONNEEDED =
    "user/subscriptionNeeded";
constexpr const char* SETTINGS_CURRENTSERVER_COUNTRYCODE =
    "currentServer/countryCode";
constexpr const char* SETTINGS_CURRENTSERVER_COUNTRY = "currentServer/country";
constexpr const char* SETTINGS_CURRENTSERVER_CITY = "currentServer/city";
constexpr const char* SETTINGS_DEVICES = "devices";
constexpr const char* SETTINGS_IAPPRODUCTS = "iapProducts";
constexpr const char* SETTINGS_CAPTIVEPORTALIPV4ADDRESSES =
    "captivePortal/ipv4Addresses";
constexpr const char* SETTINGS_CAPTIVEPORTALIPV6ADDRESSES =
    "captivePortal/ipv6Addresses";
constexpr const char* SETTINGS_POSTAUTHENTICATIONSHOWN =
    "postAuthenticationShown";
constexpr const char* SETTINGS_PROTECTSELECTEDAPPS = "protectSelectedApps";
constexpr const char* SETTINGS_VPNDISABLEDAPPS = "vpnDisabledApps";

#ifdef MVPN_IOS
constexpr const char* SETTINGS_NATIVEIOSDATAMIGRATED = "nativeIOSDataMigrated";
constexpr const char* SETTINGS_SUBSCRIPTIONTRANSACTIONS =
    "subscriptionTransactions";
#endif

#ifdef MVPN_ANDROID
constexpr const char* SETTINGS_NATIVEANDROIDSDATAMIGRATED =
    "nativeAndroidDataMigrated";
#endif

#ifdef MVPN_WINDOWS
constexpr const char* SETTINGS_NATIVEWINDOWSDATAMIGRATED =
    "nativeWindowsDataMigrated";
#endif

namespace {
Logger logger(LOG_MAIN, "SettingsHolder");

SettingsHolder* s_instance = nullptr;
}  // namespace

// static
SettingsHolder* SettingsHolder::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

#ifndef UNIT_TEST
const QSettings::Format MozFormat = QSettings::registerFormat(
    "moz", CryptoSettings::readFile, CryptoSettings::writeFile);
#endif

SettingsHolder::SettingsHolder()
    :
#ifndef UNIT_TEST
      m_settings(MozFormat, QSettings::UserScope, "mozilla", "vpn")
#else
      m_settings("mozilla_testing", "vpn")
#endif
{
  MVPN_COUNT_CTOR(SettingsHolder);

  logger.log() << "Creating SettingsHolder instance";

  Q_ASSERT(!s_instance);
  s_instance = this;
}

SettingsHolder::~SettingsHolder() {
  MVPN_COUNT_DTOR(SettingsHolder);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;

#ifdef UNIT_TEST
  m_settings.clear();
#endif
}

void SettingsHolder::clear() {
  logger.log() << "Clean up the settings";

  m_settings.remove(SETTINGS_TOKEN);
  m_settings.remove(SETTINGS_SERVERS);
  m_settings.remove(SETTINGS_PRIVATEKEY);
  m_settings.remove(SETTINGS_USER_AVATAR);
  m_settings.remove(SETTINGS_USER_DISPLAYNAME);
  m_settings.remove(SETTINGS_USER_EMAIL);
  m_settings.remove(SETTINGS_USER_MAXDEVICES);
  m_settings.remove(SETTINGS_USER_SUBSCRIPTIONNEEDED);
  m_settings.remove(SETTINGS_CURRENTSERVER_COUNTRYCODE);
  m_settings.remove(SETTINGS_CURRENTSERVER_COUNTRY);
  m_settings.remove(SETTINGS_CURRENTSERVER_CITY);
  m_settings.remove(SETTINGS_DEVICES);
  m_settings.remove(SETTINGS_IAPPRODUCTS);
  m_settings.remove(SETTINGS_POSTAUTHENTICATIONSHOWN);

  // We do not remove language, ipv6 and localnetwork settings.
}

#define GETSETDEFAULT(def, type, toType, key, has, get, set, signal)    \
  bool SettingsHolder::has() const { return m_settings.contains(key); } \
  type SettingsHolder::get() const {                                    \
    if (!has()) {                                                       \
      return def;                                                       \
    }                                                                   \
    return m_settings.value(key).toType();                              \
  }                                                                     \
  void SettingsHolder::set(const type& value) {                         \
    logger.log() << "Setting" << key << "to" << value;                  \
    m_settings.setValue(key, value);                                    \
    emit signal(value);                                                 \
  }

GETSETDEFAULT(SETTINGS_IPV6ENABLED_DEFAULT, bool, toBool, SETTINGS_IPV6ENABLED,
              hasIpv6Enabled, ipv6Enabled, setIpv6Enabled, ipv6EnabledChanged)
GETSETDEFAULT(SETTINGS_LOCALNETWORKACCESS_DEFAULT, bool, toBool,
              SETTINGS_LOCALNETWORKACCESS, hasLocalNetworkAccess,
              localNetworkAccess, setLocalNetworkAccess,
              localNetworkAccessChanged)
GETSETDEFAULT(SETTINGS_UNSECUREDNETWORKALERT_DEFAULT, bool, toBool,
              SETTINGS_UNSECUREDNETWORKALERT, hasUnsecuredNetworkAlert,
              unsecuredNetworkAlert, setUnsecuredNetworkAlert,
              unsecuredNetworkAlertChanged)
GETSETDEFAULT(SETTINGS_CAPTIVEPORTALALERT_DEFAULT, bool, toBool,
              SETTINGS_CAPTIVEPORTALALERT, hasCaptivePortalAlert,
              captivePortalAlert, setCaptivePortalAlert,
              captivePortalAlertChanged)
GETSETDEFAULT(SETTINGS_STARTATBOOT_DEFAULT, bool, toBool, SETTINGS_STARTATBOOT,
              hasStartAtBoot, startAtBoot, setStartAtBoot, startAtBootChanged)
GETSETDEFAULT(QString(), QString, toString, SETTINGS_LANGUAGECODE,
              hasLanguageCode, languageCode, setLanguageCode,
              languageCodeChanged)
GETSETDEFAULT(SETTINGS_PROTECTSELECTEDAPPS_DEFAULT, bool, toBool,
              SETTINGS_PROTECTSELECTEDAPPS, hasProtectSelectedApps,
              protectSelectedApps, setProtectSelectedApps,
              protectSelectedAppsChanged)

#undef GETSETDEFAULT

#define GETSET(type, toType, key, has, get, set)                        \
  bool SettingsHolder::has() const { return m_settings.contains(key); } \
  type SettingsHolder::get() const {                                    \
    Q_ASSERT(has());                                                    \
    return m_settings.value(key).toType();                              \
  }                                                                     \
  void SettingsHolder::set(const type& value) {                         \
    logger.log() << "Setting" << key;                                   \
    m_settings.setValue(key, value);                                    \
  }

GETSET(QString, toString, SETTINGS_TOKEN, hasToken, token, setToken)
GETSET(QString, toString, SETTINGS_PRIVATEKEY, hasPrivateKey, privateKey,
       setPrivateKey)
GETSET(QByteArray, toByteArray, SETTINGS_SERVERS, hasServers, servers,
       setServers)
GETSET(QString, toString, SETTINGS_USER_AVATAR, hasUserAvatar, userAvatar,
       setUserAvatar)
GETSET(QString, toString, SETTINGS_USER_DISPLAYNAME, hasUserDisplayName,
       userDisplayName, setUserDisplayName)
GETSET(QString, toString, SETTINGS_USER_EMAIL, hasUserEmail, userEmail,
       setUserEmail)
GETSET(int, toInt, SETTINGS_USER_MAXDEVICES, hasUserMaxDevices, userMaxDevices,
       setUserMaxDevices)
GETSET(bool, toBool, SETTINGS_USER_SUBSCRIPTIONNEEDED,
       hasUserSubscriptionNeeded, userSubscriptionNeeded,
       setUserSubscriptionNeeded)
GETSET(QString, toString, SETTINGS_CURRENTSERVER_COUNTRYCODE,
       hasCurrentServerCountryCode, currentServerCountryCode,
       setCurrentServerCountryCode)
GETSET(QString, toString, SETTINGS_CURRENTSERVER_COUNTRY,
       hasCurrentServerCountry, currentServerCountry, setCurrentServerCountry)
GETSET(QString, toString, SETTINGS_CURRENTSERVER_CITY, hasCurrentServerCity,
       currentServerCity, setCurrentServerCity)
GETSET(QByteArray, toByteArray, SETTINGS_DEVICES, hasDevices, devices,
       setDevices)
GETSET(QStringList, toStringList, SETTINGS_IAPPRODUCTS, hasIapProducts,
       iapProducts, setIapProducts)
GETSET(QStringList, toStringList, SETTINGS_CAPTIVEPORTALIPV4ADDRESSES,
       hasCaptivePortalIpv4Addresses, captivePortalIpv4Addresses,
       setCaptivePortalIpv4Addresses)
GETSET(QStringList, toStringList, SETTINGS_CAPTIVEPORTALIPV6ADDRESSES,
       hasCaptivePortalIpv6Addresses, captivePortalIpv6Addresses,
       setCaptivePortalIpv6Addresses)
GETSET(bool, toBool, SETTINGS_POSTAUTHENTICATIONSHOWN,
       hasPostAuthenticationShown, postAuthenticationShown,
       setPostAuthenticationShown);

GETSET(QStringList, toStringList, SETTINGS_VPNDISABLEDAPPS, hasVpnDisabledApps,
       vpnDisabledApps, setVpnDisabledApps)
#ifdef MVPN_ANDROID
GETSET(bool, toBool, SETTINGS_NATIVEANDROIDSDATAMIGRATED,
       hasNativeAndroidDataMigrated, nativeAndroidDataMigrated,
       setNativeAndroidDataMigrated);
#endif

#ifdef MVPN_IOS
GETSET(bool, toBool, SETTINGS_NATIVEIOSDATAMIGRATED, hasNativeIOSDataMigrated,
       nativeIOSDataMigrated, setNativeIOSDataMigrated)
GETSET(QStringList, toStringList, SETTINGS_SUBSCRIPTIONTRANSACTIONS,
       hasSubscriptionTransactions, subscriptionTransactions,
       setSubscriptionTransactions)

bool SettingsHolder::hasSubscriptionTransaction(
    const QString& transactionId) const {
  return hasSubscriptionTransactions() &&
         subscriptionTransactions().contains(transactionId);
}

void SettingsHolder::addSubscriptionTransactions(
    const QStringList& transactionIds) {
  QStringList transactions;
  if (hasSubscriptionTransactions()) {
    transactions = subscriptionTransactions();
  }

  transactions.append(transactionIds);
  setSubscriptionTransactions(transactions);
}
#endif

#ifdef MVPN_WINDOWS
GETSET(bool, toBool, SETTINGS_NATIVEWINDOWSDATAMIGRATED,
       hasNativeWindowsDataMigrated, nativeWindowsDataMigrated,
       setNativeWindowsDataMigrated)
#endif

#undef GETSET
