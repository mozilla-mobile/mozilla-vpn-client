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
  Q_PROPERTY(QString placeholderUserDNS READ placeholderUserDNS CONSTANT)
  Q_PROPERTY(bool developerUnlock READ developerUnlock WRITE setDeveloperUnlock
                 NOTIFY developerUnlockChanged)
  Q_PROPERTY(bool stagingServer READ stagingServer WRITE setStagingServer NOTIFY
                 stagingServerChanged)
  Q_PROPERTY(QStringList seenFeatures READ seenFeatures WRITE setSeenFeatures
                 NOTIFY seenFeaturesChanged)
  Q_PROPERTY(bool featuresTourShown READ featuresTourShown WRITE
                 setFeaturesTourShown NOTIFY featuresTourShownChanged)
  Q_PROPERTY(int dnsProvider READ dnsProvider WRITE setDNSProvider NOTIFY
                 dnsProviderChanged)
  Q_PROPERTY(
      QString userDNS READ userDNS WRITE setUserDNS NOTIFY userDNSChanged)
  Q_PROPERTY(QStringList recentConnections READ recentConnections WRITE
                 setRecentConnections NOTIFY recentConnectionsChanged)

 public:
  SettingsHolder();
  ~SettingsHolder();

  static SettingsHolder* instance();

  bool firstExecution() const { return m_firstExecution; }

  enum DnsProvider {
    Gateway = 0,
    BlockAll = 1,
    BlockAds = 2,
    BlockTracking = 3,
    Custom = 4,
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
  GETSET(QString, hasEntryServerCountryCode, entryServerCountryCode,
         setEntryServerCountryCode)
  GETSET(QString, hasEntryServerCountry, entryServerCountry,
         setEntryServerCountry)
  GETSET(QString, hasEntryServerCity, entryServerCity, setEntryServerCity)
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
  GETSET(QString, hasUserDNS, userDNS, setUserDNS)
  GETSET(int, hasDNSProvider, dnsProvider, setDNSProvider)
  GETSET(bool, hasGleanEnabled, gleanEnabled, setGleanEnabled)
  GETSET(bool, hasDeveloperUnlock, developerUnlock, setDeveloperUnlock)
  GETSET(bool, hasStagingServer, stagingServer, setStagingServer)
  GETSET(QDateTime, hasInstallationTime, installationTime, setInstallationTime)
  GETSET(bool, hasServerSwitchNotification, serverSwitchNotification,
         setServerSwitchNotification);
  GETSET(bool, hasConnectionChangeNotification, connectionChangeNotification,
         setConnectionChangeNotification);
  GETSET(bool, hasFeaturesTourShown, featuresTourShown, setFeaturesTourShown);
  GETSET(QStringList, hasMissingApps, missingApps, setMissingApps)
  GETSET(QStringList, hasSeenFeatures, seenFeatures, setSeenFeatures)
  GETSET(QStringList, hasDevModeFeatureFlags, devModeFeatureFlags,
         setDevModeFeatureFlags);
  GETSET(QStringList, hasRecentConnections, recentConnections,
         setRecentConnections);
  GETSET(QString, hasDeviceKeyVersion, deviceKeyVersion, setDeviceKeyVersion)

  void removeMissingApp(const QString& appID);
  void addMissingApp(const QString& appID);

  bool hasVpnDisabledApp(const QString& appID);
  void removeVpnDisabledApp(const QString& appID);
  void addVpnDisabledApp(const QString& appID);

  bool hasDevModeFeatureFlag(const QString& featureID);
  void enableDevModeFeatureFlag(const QString& featureID);
  void removeDevModeFeatureFlag(const QString& featureID);

  void addConsumedSurvey(const QString& surveyId);

  void removeEntryServer();

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

#if defined(MVPN_ADJUST)
  GETSET(bool, hasAdjustActivatable, adjustActivatable, setAdjustActivatable);
#endif

#undef GETSET

  QString placeholderUserDNS() const;

  // Delete _ALL_ the settings. Probably this method is not what you want to
  // use.
  void hardReset();

 signals:
  void ipv6EnabledChanged(bool value);
  void localNetworkAccessChanged(bool value);
  void unsecuredNetworkAlertChanged(bool value);
  void captivePortalAlertChanged(bool value);
  void startAtBootChanged(bool value);
  void protectSelectedAppsChanged(bool value);
  void vpnDisabledAppsChanged(const QStringList& apps);
  void userDNSChanged(QString value);
  void dnsProviderChanged(int value);
  void gleanEnabledChanged(bool value);
  void serverSwitchNotificationChanged(bool value);
  void connectionChangeNotificationChanged(bool value);
  void developerUnlockChanged(bool value);
  void stagingServerChanged(bool value);
  void seenFeaturesChanged(const QStringList& featureIDs);
  void featuresTourShownChanged(bool value);
  void devModeFeatureFlagsChanged(const QStringList& featureIDs);
  void entryServerCountryCodeChanged(const QString& value);
  void entryServerCityChanged(const QString& value);
  void recentConnectionsChanged(const QStringList& value);
  void telemetryPolicyShownChanged(bool value);

 private:
  explicit SettingsHolder(QObject* parent);

 private:
  QSettings m_settings;
  bool m_firstExecution = false;
};

#endif  // SETTINGSHOLDER_H
