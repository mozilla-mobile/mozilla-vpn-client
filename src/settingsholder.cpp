/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsholder.h"
#include "constants.h"
#include "cryptosettings.h"
#include "featurelist.h"
#include "leakdetector.h"
#include "logger.h"
#include "rfc/rfc1918.h"
#include "rfc/rfc4193.h"
#include "rfc/rfc4291.h"
#include "rfc/rfc5735.h"

#include <QSettings>
#include <QHostAddress>

constexpr bool SETTINGS_IPV6ENABLED_DEFAULT = true;
constexpr bool SETTINGS_LOCALNETWORKACCESS_DEFAULT = false;
constexpr bool SETTINGS_UNSECUREDNETWORKALERT_DEFAULT = true;
constexpr bool SETTINGS_CAPTIVEPORTALALERT_DEFAULT = true;
constexpr bool SETTINGS_STARTATBOOT_DEFAULT = false;
constexpr bool SETTINGS_PROTECTSELECTEDAPPS_DEFAULT = false;
constexpr bool SETTINGS_SERVERSWITCHNOTIFICATION_DEFAULT = true;
constexpr bool SETTINGS_CONNECTIONSWITCHNOTIFICATION_DEFAULT = true;
constexpr bool SETTINGS_USEGATEWAYDNS_DEFAULT = true;
const QStringList SETTINGS_VPNDISABLEDAPPS_DEFAULT = QStringList();
constexpr const char* SETTINGS_USER_DNS_DEFAULT = "";

constexpr const char* SETTINGS_IPV6ENABLED = "ipv6Enabled";
constexpr const char* SETTINGS_LOCALNETWORKACCESS = "localNetworkAccess";
constexpr const char* SETTINGS_UNSECUREDNETWORKALERT = "unsecuredNetworkAlert";
constexpr const char* SETTINGS_SERVERSWITCHNOTIFICATION =
    "serverSwitchNotification";
constexpr const char* SETTINGS_CONNECTIONSWITCHNOTIFICATION =
    "connectionChangeNotification";
constexpr const char* SETTINGS_CAPTIVEPORTALALERT = "captivePortalAlert";
constexpr const char* SETTINGS_STARTATBOOT = "startAtBoot";
constexpr const char* SETTINGS_LANGUAGECODE = "languageCode";
constexpr const char* SETTINGS_PREVIOUSLANGUAGECODE = "previousLanguageCode";
constexpr const char* SETTINGS_SYSTEMLANGUAGECODEMIGRATED =
    "systemLanguageCodeMigrated";
constexpr const char* SETTINGS_INSTALLATIONTIME = "installationTime";
constexpr const char* SETTINGS_TOKEN = "token";
constexpr const char* SETTINGS_SERVERS = "servers";
constexpr const char* SETTINGS_PRIVATEKEY = "privateKey";
constexpr const char* SETTINGS_PUBLICKEY = "publicKey";
constexpr const char* SETTINGS_USEGATEWAYDNS = "useGatewayDNS";
constexpr const char* SETTINGS_USER_AVATAR = "user/avatar";
constexpr const char* SETTINGS_USER_DNS = "user/dns";
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
constexpr const char* SETTINGS_SURVEYS = "surveys";
constexpr const char* SETTINGS_CONSUMEDSURVEYS = "consumedSurveys";
constexpr const char* SETTINGS_IAPPRODUCTS = "iapProducts";
constexpr const char* SETTINGS_CAPTIVEPORTALIPV4ADDRESSES =
    "captivePortal/ipv4Addresses";
constexpr const char* SETTINGS_CAPTIVEPORTALIPV6ADDRESSES =
    "captivePortal/ipv6Addresses";
constexpr const char* SETTINGS_POSTAUTHENTICATIONSHOWN =
    "postAuthenticationShown";
constexpr const char* SETTINGS_TELEMETRYPOLICYSHOWN = "telemetryPolicyShown";
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

constexpr const char* SETTINGS_WIN_MISSING_SPLITTUNNEL_APPS = "winMissingApps";
#endif

constexpr bool SETTINGS_GLEANENABLED_DEFAULT = true;
constexpr const char* SETTINGS_GLEANENABLED = "gleanEnabled";

namespace {
Logger logger(LOG_MAIN, "SettingsHolder");
// Setting Keys That won't show up in a report;
QVector<QString> SENSITIVE_SETTINGS({
    SETTINGS_TOKEN, SETTINGS_PRIVATEKEY,
    SETTINGS_SERVERS,  // Those 2 Are not sensitive but
    SETTINGS_DEVICES   // are more noise then info
});

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

  logger.debug() << "Creating SettingsHolder instance";

  Q_ASSERT(!s_instance);
  s_instance = this;

  if (!hasInstallationTime()) {
    setInstallationTime(QDateTime::currentDateTime());
  }
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
  logger.debug() << "Clean up the settings";

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
  m_settings.remove(SETTINGS_SURVEYS);
  m_settings.remove(SETTINGS_IAPPRODUCTS);
  m_settings.remove(SETTINGS_POSTAUTHENTICATIONSHOWN);

  // We do not remove language, ipv6 and localnetwork settings.
}

// Returns a Report which settings are set
// Used to Print in LogFiles:
QString SettingsHolder::getReport() {
  QString buff;
  QTextStream out(&buff);
  auto settingsKeys = m_settings.childKeys();
  for (auto setting : settingsKeys) {
    if (SENSITIVE_SETTINGS.contains(setting)) {
      out << setting << " -> <Sensitive>" << Qt::endl;
      continue;
    }
    out << setting << " -> " << m_settings.value(setting).toString()
        << Qt::endl;
  }
  return buff;
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
    logger.debug() << "Setting" << key << "to" << value;                \
    m_settings.setValue(key, value);                                    \
    emit signal(value);                                                 \
  }

GETSETDEFAULT(SETTINGS_IPV6ENABLED_DEFAULT, bool, toBool, SETTINGS_IPV6ENABLED,
              hasIpv6Enabled, ipv6Enabled, setIpv6Enabled, ipv6EnabledChanged)
GETSETDEFAULT(FeatureList::instance()->localNetworkAccessSupported() &&
                  SETTINGS_LOCALNETWORKACCESS_DEFAULT,
              bool, toBool, SETTINGS_LOCALNETWORKACCESS, hasLocalNetworkAccess,
              localNetworkAccess, setLocalNetworkAccess,
              localNetworkAccessChanged)
GETSETDEFAULT(SETTINGS_USEGATEWAYDNS_DEFAULT, bool, toBool,
              SETTINGS_USEGATEWAYDNS, hasUsegatewayDNS, useGatewayDNS,
              setUseGatewayDNS, useGatewayDNSChanged)
GETSETDEFAULT(SETTINGS_USER_DNS_DEFAULT, QString, toString, SETTINGS_USER_DNS,
              hasUserDNS, userDNS, setUserDNS, userDNSChanged)
GETSETDEFAULT(
    FeatureList::instance()->unsecuredNetworkNotificationSupported() &&
        SETTINGS_UNSECUREDNETWORKALERT_DEFAULT,
    bool, toBool, SETTINGS_UNSECUREDNETWORKALERT, hasUnsecuredNetworkAlert,
    unsecuredNetworkAlert, setUnsecuredNetworkAlert,
    unsecuredNetworkAlertChanged)
GETSETDEFAULT(FeatureList::instance()->captivePortalNotificationSupported() &&
                  SETTINGS_CAPTIVEPORTALALERT_DEFAULT,
              bool, toBool, SETTINGS_CAPTIVEPORTALALERT, hasCaptivePortalAlert,
              captivePortalAlert, setCaptivePortalAlert,
              captivePortalAlertChanged)
GETSETDEFAULT(FeatureList::instance()->startOnBootSupported() &&
                  SETTINGS_STARTATBOOT_DEFAULT,
              bool, toBool, SETTINGS_STARTATBOOT, hasStartAtBoot, startAtBoot,
              setStartAtBoot, startAtBootChanged)
GETSETDEFAULT(FeatureList::instance()->protectSelectedAppsSupported() &&
                  SETTINGS_PROTECTSELECTEDAPPS_DEFAULT,
              bool, toBool, SETTINGS_PROTECTSELECTEDAPPS,
              hasProtectSelectedApps, protectSelectedApps,
              setProtectSelectedApps, protectSelectedAppsChanged)
GETSETDEFAULT(SETTINGS_VPNDISABLEDAPPS_DEFAULT, QStringList, toStringList,
              SETTINGS_VPNDISABLEDAPPS, hasVpnDisabledApps, vpnDisabledApps,
              setVpnDisabledApps, vpnDisabledAppsChanged)
GETSETDEFAULT(SETTINGS_GLEANENABLED_DEFAULT, bool, toBool,
              SETTINGS_GLEANENABLED, hasGleanEnabled, gleanEnabled,
              setGleanEnabled, gleanEnabledChanged)
GETSETDEFAULT(SETTINGS_SERVERSWITCHNOTIFICATION_DEFAULT, bool, toBool,
              SETTINGS_SERVERSWITCHNOTIFICATION, hasServerSwitchNotification,
              serverSwitchNotification, setServerSwitchNotification,
              serverSwitchNotificationChanged);
GETSETDEFAULT(SETTINGS_CONNECTIONSWITCHNOTIFICATION_DEFAULT, bool, toBool,
              SETTINGS_CONNECTIONSWITCHNOTIFICATION,
              hasConnectionChangeNotification, connectionChangeNotification,
              setConnectionChangeNotification,
              connectionChangeNotificationChanged);

#undef GETSETDEFAULT

#define GETSET(type, toType, key, has, get, set)                        \
  bool SettingsHolder::has() const { return m_settings.contains(key); } \
  type SettingsHolder::get() const {                                    \
    Q_ASSERT(has());                                                    \
    return m_settings.value(key).toType();                              \
  }                                                                     \
  void SettingsHolder::set(const type& value) {                         \
    logger.debug() << "Setting" << key;                                 \
    m_settings.setValue(key, value);                                    \
  }

GETSET(QString, toString, SETTINGS_TOKEN, hasToken, token, setToken)
GETSET(QString, toString, SETTINGS_PRIVATEKEY, hasPrivateKey, privateKey,
       setPrivateKey)
GETSET(QString, toString, SETTINGS_PUBLICKEY, hasPublicKey, publicKey,
       setPublicKey)
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
GETSET(QByteArray, toByteArray, SETTINGS_SURVEYS, hasSurveys, surveys,
       setSurveys)
GETSET(QStringList, toStringList, SETTINGS_CONSUMEDSURVEYS, hasConsumedSurveys,
       consumedSurveys, setConsumedSurveys)
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
GETSET(bool, toBool, SETTINGS_TELEMETRYPOLICYSHOWN, hasTelemetryPolicyShown,
       telemetryPolicyShown, setTelemetryPolicyShown);
GETSET(QString, toString, SETTINGS_LANGUAGECODE, hasLanguageCode, languageCode,
       setLanguageCode);
GETSET(QString, toString, SETTINGS_PREVIOUSLANGUAGECODE,
       hasPreviousLanguageCode, previousLanguageCode, setPreviousLanguageCode);
GETSET(bool, toBool, SETTINGS_SYSTEMLANGUAGECODEMIGRATED,
       hasSystemLanguageCodeMigrated, systemLanguageCodeMigrated,
       setSystemLanguageCodeMigrated);
GETSET(QDateTime, toDateTime, SETTINGS_INSTALLATIONTIME, hasInstallationTime,
       installationTime, setInstallationTime);

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

GETSET(QStringList, toStringList, SETTINGS_WIN_MISSING_SPLITTUNNEL_APPS,
       hasMissingApps, missingApps, setMissingApps)

void SettingsHolder::removeMissingApp(const QString& appID) {
  QStringList applist;
  if (hasMissingApps()) {
    applist = missingApps();
  }
  applist.removeAll(appID);
  setMissingApps(applist);
}
void SettingsHolder::addMissingApp(const QString& appID) {
  QStringList applist;
  if (hasMissingApps()) {
    applist = missingApps();
  }
  if (applist.contains(appID)) {
    return;
  }
  applist.append(appID);
  setMissingApps(applist);
}
#endif

#undef GETSET

bool SettingsHolder::hasVpnDisabledApp(const QString& appID) {
  QStringList applist;
  if (hasVpnDisabledApps()) {
    applist = vpnDisabledApps();
  }
  return applist.contains(appID);
}
void SettingsHolder::removeVpnDisabledApp(const QString& appID) {
  QStringList applist;
  if (hasVpnDisabledApps()) {
    applist = vpnDisabledApps();
  }
  applist.removeAll(appID);
  setVpnDisabledApps(applist);
}
void SettingsHolder::addVpnDisabledApp(const QString& appID) {
  QStringList applist;
  if (hasVpnDisabledApps()) {
    applist = vpnDisabledApps();
  }
  applist.append(appID);
  setVpnDisabledApps(applist);
}

void SettingsHolder::addConsumedSurvey(const QString& surveyId) {
  QStringList list;
  if (hasConsumedSurveys()) {
    list = consumedSurveys();
  }
  list.append(surveyId);
  setConsumedSurveys(list);
}

SettingsHolder::UserDNSValidationResult SettingsHolder::validateUserDNS(
    const QString& dns) const {
  logger.debug() << "checking -> " << dns;
  QHostAddress address = QHostAddress(dns);

  logger.debug() << "is null " << address.isNull();

  if (address.isNull()) {
    return UserDNSInvalid;
  }

  /* Currently we need to limit this to loopback and LAN IP addresses since the
   * killswitch makes sure that no dns traffic may happen to outside of lan
   */

  if (address.protocol() == QAbstractSocket::IPv4Protocol) {
    if (RFC5735::ipv4LoopbackAddressBlock().contains(address)) {
      return UserDNSOK;
    }

    for (const IPAddress& network : RFC1918::ipv4()) {
      if (network.contains(address)) {
        return UserDNSOK;
      }
    }

    return UserDNSOutOfRange;
  }

  if (address.protocol() == QAbstractSocket::IPv6Protocol) {
    if (RFC4291::ipv6LoopbackAddressBlock().contains(address)) {
      return UserDNSOK;
    }

    for (const IPAddress& network : RFC4193::ipv6()) {
      if (network.contains(address)) {
        return UserDNSOK;
      }
    }

    return UserDNSOutOfRange;
  }

  return UserDNSInvalid;
}

QString SettingsHolder::placeholderUserDNS() const {
  return Constants::PLACEHOLDER_USER_DNS;
}
