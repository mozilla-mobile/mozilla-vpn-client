/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "constants.h"

#include <QMap>
#include <QProcessEnvironment>
#include <QString>
#include <QtGlobal>

#include "settingsholder.h"
#include "version.h"

namespace {
QString s_stagingServerAddress = "";
bool s_inProduction = true;
QString s_versionOverride = "";
}  // namespace

bool Constants::inProduction() { return s_inProduction; }

const QString& Constants::getStagingServerAddress() {
  return s_stagingServerAddress;
}

void Constants::setStaging() {
  s_inProduction = false;

  // Get staging server address. If it was set to empty by the user, remove it
  // from SettingsHolder, and then query for it again to get the default
  // address.
  s_stagingServerAddress = SettingsHolder::instance()->stagingServerAddress();
  if (s_stagingServerAddress.isEmpty()) {
    SettingsHolder::instance()->removeStagingServerAddress();
    s_stagingServerAddress = SettingsHolder::instance()->stagingServerAddress();
  }
  Q_ASSERT(!s_stagingServerAddress.isEmpty());
}

void Constants::setVersionOverride(const QString& versionOverride) {
  s_versionOverride = versionOverride;
}

QString Constants::versionString() {
  if (!s_inProduction && !s_versionOverride.isEmpty()) {
    return s_versionOverride;
  }
  return QStringLiteral(APP_VERSION);
}

QString Constants::buildNumber() { return QStringLiteral(BUILD_ID); }

QString Constants::envOrDefault(const QString& name,
                                const QString& defaultValue) {
  QString env;

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains(name)) {
    env = pe.value(name);
  }

  if (env.isEmpty()) {
    return defaultValue;
  }

  return env;
}

QString Constants::apiBaseUrl() {
  if (Constants::inProduction()) {
    return Constants::API_PRODUCTION_URL;
  }

  return Constants::getStagingServerAddress();
}

QString Constants::apiUrl(ApiEndpoint endpoint) {
  static QMap<ApiEndpoint, const char*> endpoints{
      {ApiEndpoint::Account, "/api/v1/vpn/account"},
      {ApiEndpoint::Adjust, "/api/v1/vpn/adjust"},
      {ApiEndpoint::CreateSupportTicket, "/api/v1/vpn/createSupportTicket"},
      {ApiEndpoint::CreateSupportTicketGuest,
       "/api/v1/vpn/createGuestSupportTicket"},
      {ApiEndpoint::Device, "/api/v1/vpn/device"},
      {ApiEndpoint::DeviceWithPublicKeyArgument, "/api/v1/vpn/device/%1"},
      {ApiEndpoint::DNSDetectPortal, "/api/v1/vpn/dns/detectportal"},
      {ApiEndpoint::FeatureList, "/api/v2/vpn/featurelist"},
      {ApiEndpoint::Heartbeat, "/__heartbeat__"},
      {ApiEndpoint::IPInfo, "/api/v1/vpn/ipinfo"},
      {ApiEndpoint::LoginVerify, "/api/v2/vpn/login/verify"},
#ifdef MZ_ANDROID
      {ApiEndpoint::PurchasesAndroid, "/api/v1/vpn/purchases/android"},
#endif
#ifdef MZ_IOS
      {ApiEndpoint::PurchasesIOS, "/api/v1/vpn/purchases/ios"},
#endif
#ifdef MZ_WASM
      {ApiEndpoint::PurchasesWasm, "/api/v1/vpn/purchases/wasm"},
#endif
      {ApiEndpoint::RedirectPrivacy, "/r/vpn/privacy"},
      {ApiEndpoint::RedirectSubscriptionBlocked, "/r/vpn/subscriptionBlocked"},
      {ApiEndpoint::RedirectTermsOfService, "/r/vpn/terms"},
      {ApiEndpoint::RedirectUpdateWithPlatformArgument, "/r/vpn/update/%1"},
      {ApiEndpoint::Servers, "/api/v1/vpn/servers"},
      {ApiEndpoint::SubscriptionDetails, "/api/v1/vpn/subscriptionDetails"},
      {ApiEndpoint::Versions, "/api/v1/vpn/versions"}};

  Q_ASSERT(endpoints.contains(endpoint));

  QString apiBaseUrl = Constants::apiBaseUrl();
  return apiBaseUrl.append(endpoints[endpoint]);
}
