/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "urlopener.h"

#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

#include "appconstants.h"
#include "feature.h"
#include "inspector/inspectorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"

#ifdef MZ_ANDROID
constexpr const char* GOOGLE_PLAYSTORE_URL =
    "https://play.google.com/store/apps/details?id=org.mozilla.firefox.vpn";
#endif

#ifdef MZ_IOS
constexpr const char* APPLE_STORE_URL =
    "https://apps.apple.com/us/app/mozilla-vpn-secure-private/id1489407738";
constexpr const char* APPLE_STORE_REVIEW_URL =
    "https://apps.apple.com/app/id1489407738?action=write-review";
#endif

namespace {
Logger logger("UrlOpener");
UrlOpener* s_instance = nullptr;
}  // namespace

// static
UrlOpener* UrlOpener::instance() {
  if (!s_instance) {
    s_instance = new UrlOpener(qApp);
  }
  return s_instance;
}

UrlOpener::UrlOpener(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(UrlOpener);
}

UrlOpener::~UrlOpener() { MZ_COUNT_DTOR(UrlOpener); }

void UrlOpener::openLink(LinkType linkType) {
  logger.debug() << "Opening link: " << linkType;

  QString url;
  bool addEmailAddress = false;

  switch (linkType) {
    case LinkAccount:
      url = AppConstants::fxaUrl();
      addEmailAddress = true;
      break;

    case LinkContact:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/contact");
      break;

    case LinkForgotPassword:
      url = AppConstants::fxaUrl();
      url.append("/reset_password");
      break;

    case LinkSumo:
      url = AppConstants::MOZILLA_VPN_SUMO_URL;
      break;

    case LinkLeaveReview:
      Q_ASSERT(Feature::get(Feature::Feature_appReview)->isSupported());
      url =
#if defined(MZ_IOS)
          APPLE_STORE_REVIEW_URL;
#elif defined(MZ_ANDROID)
          GOOGLE_PLAYSTORE_URL;
#else
          "";
#endif
      break;

    case LinkTermsOfService:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/terms");
      break;

    case LinkPrivacyNotice:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/privacy");
      break;

    case LinkUpdate:
#if defined(MZ_IOS)
      url = APPLE_STORE_URL;
#elif defined(MZ_ANDROID)
      url = GOOGLE_PLAYSTORE_URL;
#else
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/update/");
      url.append(Constants::PLATFORM_NAME);
#endif
      break;

    case LinkSubscriptionBlocked:
      url = NetworkRequest::apiBaseUrl();
      url.append("/r/vpn/subscriptionBlocked");
      break;
    case LinkSplitTunnelHelp:
      // TODO: This should link to a more helpful article
      url =
          "https://support.mozilla.org/kb/"
          "split-tunneling-use-mozilla-vpn-specific-apps-wind";
      break;
    case LinkInspector:
      Q_ASSERT(!Constants::inProduction());
      url = "https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/";
      break;
    case LinkCaptivePortal:
      url = QString("http://%1/success.txt")
                .arg(SettingsHolder::instance()
                         ->captivePortalIpv4Addresses()
                         .first());
      break;

    case LinkRelayPremium:
      url = AppConstants::relayUrl();
      url.append("/premium");
      break;

    case LinkSubscriptionFxa:
      url = AppConstants::fxaUrl();
      url.append("/subscriptions");
      break;

    case LinkSubscriptionIapApple:
      url = AppConstants::APPLE_SUBSCRIPTIONS_URL;
      break;

    case LinkSubscriptionIapGoogle:
      url = AppConstants::GOOGLE_SUBSCRIPTIONS_URL;
      break;

    case LinkUpgradeToBundle:
      url = Constants::inProduction() ? AppConstants::API_PRODUCTION_URL
                                      : AppConstants::API_STAGING_URL;
      url.append("/r/vpn/upgradeToPrivacyBundle");
      break;

    default:
      qFatal("Unsupported link type!");
      return;
  }

  open(url, addEmailAddress);
}

void UrlOpener::openUrl(const QString& linkUrl) {
  logger.debug() << "Opening link: " << linkUrl;
  open(linkUrl);
}

void UrlOpener::open(QUrl url, bool addEmailAddress) {
  if (addEmailAddress) {
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    if (settingsHolder->hasUserEmail()) {
      QUrlQuery query(url.query());
      query.addQueryItem("email", settingsHolder->userEmail());
      url.setQuery(query);
    }
  }

  url = replaceUrlParams(url);

  if (!Constants::inProduction()) {
    setLastUrl(url.toString());

    if (InspectorHandler::stealUrls()) {
      return;
    }
  }

  QDesktopServices::openUrl(url);
}

// static
QUrl UrlOpener::replaceUrlParams(const QUrl& originalUrl) {
  if (!originalUrl.isValid()) {
    logger.error() << "Invalid survey URL";
    return originalUrl;
  }

  QUrl url(originalUrl);
  QUrlQuery currentQuery(url.query());
  QUrlQuery newQuery;

  for (QPair<QString, QString>& item : currentQuery.queryItems()) {
    if (item.second == "__VPN_VERSION__") {
      newQuery.addQueryItem(item.first, Env::versionString());
    } else if (item.second == "__VPN_BUILDNUMBER__") {
      newQuery.addQueryItem(item.first, Env::buildNumber());
    } else if (item.second == "__VPN_OS__") {
      newQuery.addQueryItem(item.first, Env::osVersion());
    } else if (item.second == "__VPN_PLATFORM__") {
      newQuery.addQueryItem(item.first, Env::platform());
    } else if (item.second == "__VPN_ARCH__") {
      newQuery.addQueryItem(item.first, Env::architecture());
    } else if (item.second == "__VPN_GRAPHICSAPI__") {
      newQuery.addQueryItem(item.first, MozillaVPN::graphicsApi());
    } else {
      newQuery.addQueryItem(item.first, item.second);
    }
  }

  url.setQuery(newQuery);
  return url;
}
