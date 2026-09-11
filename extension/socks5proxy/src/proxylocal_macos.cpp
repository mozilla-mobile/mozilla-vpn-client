/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/socket.h>
#include <sys/un.h>

#include "proxyconnection.h"

extern "C" {
#include <CoreFoundation/CoreFoundation.h>
#include <Security/SecTask.h>
};

#include <QLocalSocket>
#include <QScopeGuard>

// static
QString ProxyConnection::localClientName(QLocalSocket* s) {
  // Fetch the audit token of the peer process.
  audit_token_t token;
  socklen_t len = sizeof(audit_token_t);
  int sd = s->socketDescriptor();
  if (getsockopt(sd, SOL_LOCAL, LOCAL_PEERTOKEN, &token, &len) != 0) {
    return QString();
  }

  // Get the sending task.
  SecTaskRef task = SecTaskCreateWithAuditToken(NULL, token);
  if (task == NULL) {
    return QString();
  }
  auto taskGuard = qScopeGuard([task]() { CFRelease(task); });

  // Get the signing identifier.
  CFStringRef id = SecTaskCopySigningIdentifier(task, NULL);
  if (id == NULL) {
    return QString();
  }
  auto stringGuard = qScopeGuard([id]() { CFRelease(id); });
  return QString::fromCFString(id);
}
