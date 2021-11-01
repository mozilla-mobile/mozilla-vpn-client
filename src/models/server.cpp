/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "server.h"
#include "leakdetector.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QRandomGenerator>

Server::Server() { MVPN_COUNT_CTOR(Server); }

Server::Server(const Server& other) {
  MVPN_COUNT_CTOR(Server);
  *this = other;
}

Server& Server::operator=(const Server& other) {
  if (this == &other) return *this;

  m_hostname = other.m_hostname;
  m_ipv4AddrIn = other.m_ipv4AddrIn;
  m_ipv4Gateway = other.m_ipv4Gateway;
  m_ipv6AddrIn = other.m_ipv6AddrIn;
  m_ipv6Gateway = other.m_ipv6Gateway;
  m_portRanges = other.m_portRanges;
  m_publicKey = other.m_publicKey;
  m_weight = other.m_weight;
  m_socksName = other.m_socksName;
  m_multihopPort = other.m_multihopPort;

  return *this;
}

Server::~Server() { MVPN_COUNT_DTOR(Server); }

bool Server::fromJson(const QJsonObject& obj) {
  // Reset.
  m_hostname = "";

  QJsonValue hostname = obj.value("hostname");
  if (!hostname.isString()) {
    return false;
  }

  QJsonValue ipv4AddrIn = obj.value("ipv4_addr_in");
  if (!ipv4AddrIn.isString()) {
    return false;
  }

  QJsonValue ipv4Gateway = obj.value("ipv4_gateway");
  if (!ipv4Gateway.isString()) {
    return false;
  }

  QJsonValue ipv6AddrIn = obj.value("ipv6_addr_in");
  // If this object comes from the IOS migration, the ipv6_addr_in is missing.

  QJsonValue ipv6Gateway = obj.value("ipv6_gateway");
  if (!ipv6Gateway.isString()) {
    return false;
  }

  QJsonValue publicKey = obj.value("public_key");
  if (!publicKey.isString()) {
    return false;
  }

  QJsonValue weight = obj.value("weight");
  if (!weight.isDouble()) {
    return false;
  }

  QJsonValue portRanges = obj.value("port_ranges");
  if (!portRanges.isArray()) {
    return false;
  }

  QJsonValue socks5_name = obj.value("socks5_name");
  if (!socks5_name.isString()) {
    return false;
  }

  QJsonValue multihop_port = obj.value("multihop_port");
  if (!multihop_port.isDouble()) {
    return false;
  }

  QList<QPair<uint32_t, uint32_t>> prList;
  QJsonArray portRangesArray = portRanges.toArray();
  for (QJsonValue portRangeValue : portRangesArray) {
    if (!portRangeValue.isArray()) {
      return false;
    }

    QJsonArray port = portRangeValue.toArray();
    if (port.count() != 2) {
      return false;
    }

    QJsonValue a = port.at(0);
    if (!a.isDouble()) {
      return false;
    }

    QJsonValue b = port.at(1);
    if (!b.isDouble()) {
      return false;
    }

    prList.append(QPair<uint32_t, uint32_t>(a.toInt(), b.toInt()));
  }

  m_hostname = hostname.toString();
  m_ipv4AddrIn = ipv4AddrIn.toString();
  m_ipv4Gateway = ipv4Gateway.toString();
  m_ipv6AddrIn = ipv6AddrIn.toString();
  m_ipv6Gateway = ipv6Gateway.toString();
  m_portRanges.swap(prList);
  m_publicKey = publicKey.toString();
  m_weight = weight.toInt();
  m_socksName = socks5_name.toString();
  m_multihopPort = multihop_port.toInt();

  return true;
}

// static
const Server& Server::weightChooser(const QList<Server>& servers) {
  static const Server emptyServer;
  Q_ASSERT(!emptyServer.initialized());
  if (servers.isEmpty()) {
    return emptyServer;
  }

  uint32_t weightSum = 0;

  for (const Server& server : servers) {
    weightSum += server.weight();
  }

  quint32 r = QRandomGenerator::global()->generate() % (weightSum + 1);

  for (const Server& server : servers) {
    if (server.weight() >= r) {
      return server;
    }

    r -= server.weight();
  }

  // This should not happen.
  Q_ASSERT(false);
  return emptyServer;
}

uint32_t Server::choosePort() const {
  if (m_portRanges.isEmpty()) {
    return 0;
  }

  // Count the total number of potential ports.
  quint32 length = 0;
  for (const QPair<uint32_t, uint32_t>& range : m_portRanges) {
    Q_ASSERT(range.first <= range.second);
    length += range.second - range.first + 1;
  }
  Q_ASSERT(length < 65536);

  while (true) {
    // Pick a port at random.
    quint32 r = QRandomGenerator::global()->generate() % length;
    quint32 port = 0;

    for (const QPair<uint32_t, uint32_t>& range : m_portRanges) {
      if (r <= (range.second - range.first)) {
        port = r + range.first;
        break;
      }
      r -= (range.second - range.first + 1);
    }
    Q_ASSERT(port != 0);

    // Avoid using port 53/DNS for multihop, since Mullvad blocks DNS traffic.
    if (port != 53) {
      return port;
    }
  }

  // We should not get here.
  Q_ASSERT(false);
  return 0;
}
