/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "linuxbypass.h"

#include <errno.h>
#include <sys/capability.h>
#include <sys/socket.h>
#include <unistd.h>

#include <QAbstractSocket>
#include <QHostAddress>
#include <QScopeGuard>

#include "socks5.h"

LinuxBypass::LinuxBypass(Socks5* proxy) : QObject(proxy) {
  cap_t caps = cap_init();
  if (caps == nullptr) {
    qWarning() << "Failed to init process capabilities";
    return;
  }
  auto guard = qScopeGuard([&] { cap_free(caps); });

  // Try to acquire CAP_NET_RAW in order to set the firewall mark.
  cap_value_t newcaps[] = {CAP_NET_RAW};
  const int numcaps = sizeof(newcaps) / sizeof(cap_value_t);
  cap_set_flag(caps, CAP_EFFECTIVE, numcaps, newcaps, CAP_SET);
  cap_set_flag(caps, CAP_PERMITTED, numcaps, newcaps, CAP_SET);
  if (cap_set_proc(caps) != 0) {
    qWarning() << "Failed to set CAP_NET_RAW: disabling split tunnel";
    cap_clear(caps);
    cap_set_proc(caps);
    return;
  }

  // If we get this far - we can perform split tunneling.
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
