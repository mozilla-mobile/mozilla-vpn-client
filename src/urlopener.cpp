/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "urlopener.h"
#include "logger.h"
#include "inspector/inspectorwebsocketconnection.h"

#include <QDesktopServices>
#include <QUrl>

namespace {
Logger logger(LOG_MAIN, "UrlOpener");
}

// static
void UrlOpener::open(const QUrl& url) {
#ifdef MVPN_INSPECTOR
  InspectorWebSocketConnection::setLastUrl(url.toString());

  if (InspectorWebSocketConnection::stealUrls()) {
    return;
  }
#endif

  QDesktopServices::openUrl(url);
}
