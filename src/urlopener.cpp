/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "urlopener.h"
#include "constants.h"
#include "logger.h"
#include "inspector/inspectorwebsocketconnection.h"
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

  if (!Constants::inProduction()) {
    InspectorWebSocketConnection::setLastUrl(url.toString());

    if (InspectorWebSocketConnection::stealUrls()) {
      return;
    }
  }

  QDesktopServices::openUrl(url);
}
