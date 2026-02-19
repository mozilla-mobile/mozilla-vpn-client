/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/socket.h>
#include <sys/un.h>

#include <QDir>
#include <QFile>
#include <QProcessEnvironment>

#include "server.h"

enum LinuxSecurityModule {
  Unknown = 0,
  SELinux,
  AppArmor,
  Disabled,
};

static QString readFromFile(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
    return QString();
  }
  QString result = QString::fromUtf8(file.readAll());
  qsizetype end = result.indexOf('\0');
  if (end >= 0) {
    result.truncate(end);
  }
  return result.trimmed();
}

bool WebExtension::Server::isAllowedToConnect(qintptr sd) {
  // Determine the major security module in use.
  static enum LinuxSecurityModule lsm = LinuxSecurityModule::Unknown;
  if (lsm == LinuxSecurityModule::Unknown) {
    QStringList modules = readFromFile("/sys/kernel/security/lsm").split(',');
    if (modules.contains("selinux")) {
      lsm = SELinux;
    } else if (modules.contains("apparmor")) {
      lsm = AppArmor;
    } else {
      lsm = Disabled;
    }
  }
  if (lsm == Disabled) {
    return true;
  }

  // Fetch the peer security context. 
  char peersec[NAME_MAX];
  socklen_t peerlen = sizeof(peersec);
  if (getsockopt(sd, SOL_SOCKET, SO_PEERSEC, peersec, &peerlen) < 0) {
    return false;
  }
  peersec[peerlen] = '\0';

  // Restrict socket access to applications running within same security policy.
  QString peer = QString(peersec);
  QString policy = readFromFile("/proc/self/attr/current");
  if (lsm == AppArmor) {
    if (policy == "unconfined") {
      return true;
    }
    return peer == policy;
  } else if (lsm == SELinux) {
    QString setype = policy.split(':').value(2);
    QString peertype = peer.split(':').value(2);
    if (setype == "unconfined_t") {
      return true;
    }
    return peertype == setype;
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
