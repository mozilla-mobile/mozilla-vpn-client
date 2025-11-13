/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "server.h"

#include <QDir>
#include <QStandardPaths>

bool WebExtension::Server::isAllowedToConnect(qintptr sd) {
  // TODO: Implement Me!
  return true;
}

QString WebExtension::Server::localSocketName() {
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
  return dir.absoluteFilePath(WEBEXT_SOCKET_NAME);
}
