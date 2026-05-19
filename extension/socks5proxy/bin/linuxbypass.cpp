/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxbypass.h"

#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include <QAbstractSocket>
#include <QDebug>
#include <QHostAddress>

#include "socks5.h"

LinuxBypass::LinuxBypass(Socks5* proxy) : QObject(proxy) {
  connect(proxy, &Socks5::outgoingConnection, this,
          &LinuxBypass::outgoingConnection);
  connect(DNSResolver::instance(), &DNSResolver::setupDnsSocket, this,
          &LinuxBypass::outgoingConnection);
}

void LinuxBypass::outgoingConnection(qintptr sd, const QHostAddress& dest) {
  constexpr const int vpn_firewall_mark = 51820;

  int err = setsockopt(sd, SOL_SOCKET, SO_MARK, &vpn_firewall_mark,
                       sizeof(vpn_firewall_mark));
  if (err != 0) {
    qWarning() << "setsockopt(SO_MARK) failed:" << strerror(errno);
  }
}
