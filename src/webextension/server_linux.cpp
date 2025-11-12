/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QDebug>
#include <QFile>

#include <sys/socket.h>
#include <sys/un.h>

#include "server.h"

bool WebExtension::Server::isAllowedToConnect(qintptr sd) {
  // Compare the socket security context to our own.
  QFile selfproc("/proc/self/attr/current");
  if (selfproc.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
    QString selfctx = QString::fromUtf8(selfproc.readAll());
    qsizetype end = selfctx.indexOf('\0');
    if (end >= 0) {
      selfctx.truncate(end);
    }

    char peersec[NAME_MAX];
    socklen_t peerlen = sizeof(peersec);
    if (getsockopt(sd, SOL_SOCKET, SO_PEERSEC, peersec, &peerlen) < 0) {
      return false;
    }
    if (QString::compare(selfctx.trimmed(), peersec) != 0) {
      return false;
    }
  }

  return true;
}
