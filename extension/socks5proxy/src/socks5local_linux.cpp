/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sys/socket.h>

#include <QFile>
#include <QLocalSocket>

#include "socks5connection.h"

// Read /proc/<pid>/cgroup and parse out the cgroupv2 control group path.
static QString lookupCgroupForPid(pid_t pid) {
  QFile cgfile(QString("/proc/%1/cgroup").arg(pid));
  if (!cgfile.exists() ||
      !cgfile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
    return QString();
  }
  while (true) {
    QString line = QString::fromUtf8(cgfile.readLine());
    if (line.isEmpty()) {
      return QString();
    }
    if (line.startsWith("0::")) {
      return line.mid(3);
    }
  }
  return QString();
}

// static
// Return the systemd scope of the process at the other end of the local socket.
QString Socks5Connection::localClientName(QLocalSocket* s) {
  struct ucred peer;
  socklen_t len = sizeof(peer);
  int sd = s->socketDescriptor();
  if (getsockopt(sd, SOL_SOCKET, SO_PEERCRED, &peer, &len) != 0) {
    return QString();
  }
  // Don't believe anyone who tells us they are PID zero.
  if (!peer.pid) {
    return QString();
  }

  QString cgscope;
  for (const QString& segment : lookupCgroupForPid(peer.pid).split("/")) {
    if (segment.endsWith(".scope")) {
      cgscope = segment;
    }
  }
  return cgscope;
}
