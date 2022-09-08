/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "urlopener.h"
#include "constants.h"
#include "logger.h"
#include "mozillavpn.h"
#include "inspector/inspectorhandler.h"
#include "settingsholder.h"

#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

namespace {
Logger logger(LOG_MAIN, "UrlOpener");
}

// static
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
    MozillaVPN* vpn = MozillaVPN::instance();
    vpn->setLastUrl(url.toString());

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
