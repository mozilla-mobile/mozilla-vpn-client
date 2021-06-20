/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QDateTime>
#include <QStringList>
#include <QObject>
#include <QSettings>

class SettingsHolder final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SettingsHolder)

  Q_PROPERTY(bool ipv6Enabled READ ipv6Enabled WRITE setIpv6Enabled NOTIFY
                 ipv6EnabledChanged)
  Q_PROPERTY(bool localNetworkAccess READ localNetworkAccess WRITE
                 setLocalNetworkAccess NOTIFY localNetworkAccessChanged)
  Q_PROPERTY(bool unsecuredNetworkAlert READ unsecuredNetworkAlert WRITE
                 setUnsecuredNetworkAlert NOTIFY unsecuredNetworkAlertChanged)
  Q_PROPERTY(bool captivePortalAlert READ captivePortalAlert WRITE
                 setCaptivePortalAlert NOTIFY captivePortalAlertChanged)
  Q_PROPERTY(bool startAtBoot READ startAtBoot WRITE setStartAtBoot NOTIFY
                 startAtBootChanged)
  Q_PROPERTY(bool protectSelectedApps READ protectSelectedApps WRITE
                 setProtectSelectedApps NOTIFY protectSelectedAppsChanged)
  Q_PROPERTY(bool gleanEnabled READ gleanEnabled WRITE setGleanEnabled NOTIFY
                 gleanEnabledChanged)
  Q_PROPERTY(
      bool serverSwitchNotification READ serverSwitchNotification WRITE
          setServerSwitchNotification NOTIFY serverSwitchNotificationChanged)
  Q_PROPERTY(bool connectionChangeNotification READ connectionChangeNotification
                 WRITE setConnectionChangeNotification NOTIFY
                     connectionChangeNotificationChanged)
  Q_PROPERTY(bool useGatewayDNS READ useGatewayDNS WRITE setUseGatewayDNS NOTIFY
                 useGatewayDNSChanged)
  Q_PROPERTY(
      QString customDNS READ customDNS WRITE setcustomDNS NOTIFY customDNSChanged)
  Q_PROPERTY(
        int dnsProvider READ dnsProvider WRITE setDNSProvider NOTIFY dnsProviderChanged)

 public:
  SettingsHolder();
  ~SettingsHolder();

  static SettingsHolder* instance();

  enum DnsProvider {
    Custom=3,
    BlockTracking=2,
    BlockAds=1,
    BlockAll=0,
  };
  Q_ENUM(DnsProvider)

  QString getReport();

  void clear();

#define GETSET(type, has, get, set) \
  bool has() const;                 \
  type get() const;                 \
  void set(const type& value);

  GETSET(bool, hasIpv6Enabled, ipv6Enabled, setIpv6Enabled)
  GETSET(bool, hasLocalNetworkAccess, localNetworkAccess, setLocalNetworkAccess)
  GETSET(bool, hasUnsecuredNetworkAlert, unsecuredNetworkAlert,
         setUnsecuredNetworkAlert)
  GETSET(bool, hasCaptivePortalAlert, captivePortalAlert, setCaptivePortalAlert)
  GETSET(bool, hasStartAtBoot, startAtBoot, setStartAtBoot)
  GETSET(QString, hasLanguageCode, languageCode, setLanguageCode)
  GETSET(QString, hasPreviousLanguageCode, previousLanguageCode,
         setPreviousLanguageCode)
  GETSET(bool, hasSystemLanguageCodeMigrated, systemLanguageCodeMigrated,
         setSystemLanguageCodeMigrated)
  GETSET(QString, hasToken, token, setToken)
  GETSET(QString, hasPrivateKey, privateKey, setPrivateKey)
  GETSET(QString, hasPublicKey, publicKey, setPublicKey)
  GETSET(QByteArray, hasServers, servers, setServers)
  GETSET(QString, hasUserAvatar, userAvatar, setUserAvatar)
  GETSET(QString, hasUserDisplayName, userDisplayName, setUserDisplayName)
  GETSET(QString, hasUserEmail, userEmail, setUserEmail)
  GETSET(int, hasUserMaxDevices, userMaxDevices, setUserMaxDevices)
  GETSET(bool, hasUserSubscriptionNeeded, userSubscriptionNeeded,
         setUserSubscriptionNeeded)
  GETSET(QString, hasCurrentServerCountryCode, currentServerCountryCode,
         setCurrentServerCountryCode)
  GETSET(QString, hasCurrentServerCountry, currentServerCountry,
         setCurrentServerCountry)
  GETSET(QString, hasCurrentServerCity, currentServerCity, setCurrentServerCity)
  GETSET(QByteArray, hasDevices, devices, setDevices)
  GETSET(QByteArray, hasSurveys, surveys, setSurveys)
  GETSET(QStringList, hasConsumedSurveys, consumedSurveys, setConsumedSurveys)
  GETSET(QStringList, hasIapProducts, iapProducts, setIapProducts)
  GETSET(QStringList, hasCaptivePortalIpv4Addresses, captivePortalIpv4Addresses,
         setCaptivePortalIpv4Addresses)
  GETSET(QStringList, hasCaptivePortalIpv6Addresses, captivePortalIpv6Addresses,
         setCaptivePortalIpv6Addresses)
  GETSET(bool, hasPostAuthenticationShown, postAuthenticationShown,
         setPostAuthenticationShown);
  GETSET(bool, hasTelemetryPolicyShown, telemetryPolicyShown,
         setTelemetryPolicyShown);
  GETSET(bool, hasProtectSelectedApps, protectSelectedApps,
         setProtectSelectedApps)
  GETSET(QStringList, hasVpnDisabledApps, vpnDisabledApps, setVpnDisabledApps)
  GETSET(bool, hasUsegatewayDNS, useGatewayDNS, setUseGatewayDNS)
  GETSET(QString, hascustomDNS, customDNS, setcustomDNS)
  GETSET(int, hasDNSProvider, dnsProvider, setDNSProvider)
  GETSET(bool, hasGleanEnabled, gleanEnabled, setGleanEnabled)
  GETSET(QDateTime, hasInstallationTime, installationTime, setInstallationTime)
  GETSET(bool, hasServerSwitchNotification, serverSwitchNotification,
         setServerSwitchNotification);
  GETSET(bool, hasConnectionChangeNotification, connectionChangeNotification,
         setConnectionChangeNotification);

  bool hasVpnDisabledApp(const QString& appID);
  void removeVpnDisabledApp(const QString& appID);
  void addVpnDisabledApp(const QString& appID);

  void addConsumedSurvey(const QString& surveyId);

  Q_INVOKABLE
  bool isValidCustomDNS(const QString& dns);
  QString getDNS(const QString& serverGateway);

#ifdef MVPN_IOS
  GETSET(bool, hasNativeIOSDataMigrated, nativeIOSDataMigrated,
         setNativeIOSDataMigrated)
  GETSET(QStringList, hasSubscriptionTransactions, subscriptionTransactions,
         setSubscriptionTransactions);

  bool hasSubscriptionTransaction(const QString& transactionId) const;
  void addSubscriptionTransactions(const QStringList& transactionIds);
#endif

#ifdef MVPN_WINDOWS
  GETSET(bool, hasNativeWindowsDataMigrated, nativeWindowsDataMigrated,
         setNativeWindowsDataMigrated)
#endif

#ifdef MVPN_ANDROID
  GETSET(bool, hasNativeAndroidDataMigrated, nativeAndroidDataMigrated,
         setNativeAndroidDataMigrated)
#endif

#undef GETSET

 signals:
  void ipv6EnabledChanged(bool value);
  void localNetworkAccessChanged(bool value);
  void unsecuredNetworkAlertChanged(bool value);
  void captivePortalAlertChanged(bool value);
  void startAtBootChanged(bool value);
  void protectSelectedAppsChanged(bool value);
  void vpnDisabledAppsChanged(const QStringList& apps);
  void useGatewayDNSChanged(bool value);
  void customDNSChanged(QString value);
  void dnsProviderChanged(int value);
  void gleanEnabledChanged(bool value);
  void serverSwitchNotificationChanged(bool value);
  void connectionChangeNotificationChanged(bool value);

 private:
  explicit SettingsHolder(QObject* parent);

 private:
  QSettings m_settings;
};

#endif  // SETTINGSHOLDER_H
