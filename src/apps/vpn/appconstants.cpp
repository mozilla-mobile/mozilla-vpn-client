/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "appconstants.h"

#include <QMap>
#include <QString>

#include "settingsholder.h"

namespace {
QString s_stagingServerAddress = "";
}  // namespace

const QString& AppConstants::getStagingServerAddress() {
  return s_stagingServerAddress;
}

void AppConstants::setStaging() {
  Constants::setStaging();

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

QString AppConstants::apiBaseUrl() {
  if (Constants::inProduction()) {
    return AppConstants::API_PRODUCTION_URL;
  }

  return AppConstants::getStagingServerAddress();
}

QString AppConstants::apiUrl(ApiEndpoint endpoint) {
  static QMap<ApiEndpoint, const char*> endpoints{
      {ApiEndpoint::Account, "/api/v1/vpn/account"},
      {ApiEndpoint::CreateSupportTicket, "/api/v1/vpn/createSupportTicket"},
      {ApiEndpoint::CreateSupportTicketGuest,
       "/api/v1/vpn/createGuestSupportTicket"},
      {ApiEndpoint::Device, "/api/v1/vpn/device"},
      {ApiEndpoint::DeviceWithPublicKeyArgument, "/api/v1/vpn/device/%1"},
      {ApiEndpoint::DNSDetectPortal, "/api/v1/vpn/dns/detectportal"},
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

  QString apiBaseUrl = AppConstants::apiBaseUrl();
  return apiBaseUrl.append(endpoints[endpoint]);
}
