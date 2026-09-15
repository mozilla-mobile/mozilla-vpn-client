/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDir>
#include <QScopeGuard>
#include <QStandardPaths>

extern "C" {
#include <CoreFoundation/CoreFoundation.h>
#include <Security/SecTask.h>
};

#include <sys/socket.h>
#include <sys/un.h>

#include "server.h"

static QString getSelfSigningIdentifier() {
  SecTaskRef task = SecTaskCreateFromSelf(NULL);
  if (!task) {
    return QString();
  }
  CFStringRef id = SecTaskCopySigningIdentifier(task, NULL);
  CFRelease(task);
  if (!id) {
    return QString();
  }
  QString result = QString::fromCFString(id);
  CFRelease(id);
  return result;
}

bool WebExtension::Server::isAllowedToConnect(qintptr sd) {
  static QString selfIdentifier = getSelfSigningIdentifier();
  if (selfIdentifier.isEmpty()) {
    // If our task is unsigned, then permit all access.
    return true;
  }

  // Fetch the audit token and signing identifier of the peer process.
  audit_token_t token;
  socklen_t len = sizeof(audit_token_t);
  if (getsockopt(sd, SOL_LOCAL, LOCAL_PEERTOKEN, &token, &len) != 0) {
    return false;
  }
  SecTaskRef task = SecTaskCreateWithAuditToken(NULL, token);
  if (!task) {
    return false;
  }
  CFStringRef id = SecTaskCopySigningIdentifier(task, NULL);
  CFRelease(task);
  if (!id) {
    return false;
  }
  auto guard = qScopeGuard([id]() { CFRelease(id); });

#ifdef MZ_DEBUG
  qDebug() << "[Debug] Allowing Webextension Connection from:"
           << QString::fromCFString(id);
  return true;
#else
  return selfIdentifier == QString::fromCFString(id);
#endif
}

QString WebExtension::Server::localSocketName() {
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
  return dir.absoluteFilePath(WEBEXT_SOCKET_NAME);
}
