/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverextra.h"
#include "leakdetector.h"

#include <QJsonObject>
#include <QJsonValue>

ServerExtra::ServerExtra() { MVPN_COUNT_CTOR(ServerExtra); }

ServerExtra::ServerExtra(const ServerExtra& other) {
  MVPN_COUNT_CTOR(ServerExtra);
  *this = other;
}

ServerExtra& ServerExtra::operator=(const ServerExtra& other) {
  if (this == &other) return *this;

  m_publicKey = other.m_publicKey;
  m_socksName = other.m_socksName;
  m_multihopPort = other.m_multihopPort;

  return *this;
}

ServerExtra::~ServerExtra() { MVPN_COUNT_DTOR(ServerExtra); }

bool ServerExtra::fromJson(const QJsonObject& obj) {
  // Reset.
  m_publicKey = "";

  QJsonValue type = obj.value("type");
  if (!type.isString() || type.toString() != "wireguard") {
    return false;
  }

  QJsonValue publicKey = obj.value("pubkey");
  if (!publicKey.isString()) {
    return false;
  }

  QJsonValue socksName = obj.value("socks_name");

  QJsonValue multihopPort = obj.value("multihop_port");
  if (!multihopPort.isDouble() && !socksName.isString()) {
    return false;
  }

  m_publicKey = publicKey.toString();
  m_socksName = socksName.toString();
  m_multihopPort = multihopPort.toInt();

  return true;
}
