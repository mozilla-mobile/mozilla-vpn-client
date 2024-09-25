/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxbypass.h"

#include <errno.h>
#include <sys/socket.h>

#include <QAbstractSocket>
#include <QHostAddress>

#include "socks5.h"

LinuxBypass::LinuxBypass(Socks5* proxy) : QObject(proxy) {
  connect(proxy, &Socks5::outgoingConnection, this,
          &LinuxBypass::outgoingConnection);
}

void LinuxBypass::outgoingConnection(QAbstractSocket* s,
                                     const QHostAddress& dest) {
  constexpr const int vpn_firewall_mark = 51820;
  int af = AF_INET;

  if (dest.protocol() == QAbstractSocket::IPv6Protocol) {
    af = AF_INET6;
  }

  // Create a socket and set its firewall mark.
  int newsock = socket(af, SOCK_STREAM, 0);
  if (newsock < 0) {
    qWarning() << "socket() failed:" << strerror(errno);
    return;
  }

  int err = setsockopt(newsock, SOL_SOCKET, SO_MARK, &vpn_firewall_mark,
                       sizeof(vpn_firewall_mark));
  if (err != 0) {
    qWarning() << "setsockopt(SO_MARK) failed:" << strerror(errno);
    close(newsock);
  } else if (!s->setSocketDescriptor(newsock,
                                     QAbstractSocket::UnconnectedState)) {
    qWarning() << "setSocketDescriptor() failed:" << s->errorString();
  }
}
