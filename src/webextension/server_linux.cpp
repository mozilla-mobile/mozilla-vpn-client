/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/socket.h>
#include <sys/un.h>

#include <QDir>
#include <QFile>
#include <QProcessEnvironment>

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
    peersec[peerlen] = '\0';
    if (QString::compare(selfctx.trimmed(), peersec) != 0) {
      return false;
    }
  }

  return true;
}

QString WebExtension::Server::localSocketName() {
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  QDir runtime(pe.value("XDG_RUNTIME_DIR"));
  if (pe.contains("FLATPAK_ID")) {
    runtime.cd("app");
    runtime.cd(pe.value("FLATPAK_ID"));
  }
  return runtime.absoluteFilePath(WEBEXT_SOCKET_NAME);
}
