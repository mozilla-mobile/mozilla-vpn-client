/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macfwpolicy.h"

#include <QLocalServer>

#include "proxyconnection.h"
#include "socks5.h"

MacFwPolicy::MacFwPolicy(Socks5* proxy) : QObject(proxy) {

  // For QLocalServers, we should check that the local client
  // name corresponds to a known web browser.
  if (qobject_cast<QLocalServer*>(proxy->parent()) != nullptr) {
    connect(proxy, &Socks5::incomingConnection, this,
            &MacFwPolicy::checkLocalSocket);
  }

  // TODO: Somehow enumerate the list of installed web browsers.
  m_browsers = QStringList({
    "com.apple.Safari",
    "org.mozilla.firefox",
    "org.mozilla.nightly",
  });
}

void MacFwPolicy::checkLocalSocket(ProxyConnection* connection) {
  if (!m_browsers.contains(connection->clientName())) {
    connection->abort();
  }
}
