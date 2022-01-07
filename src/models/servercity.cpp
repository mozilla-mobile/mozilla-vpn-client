/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercity.h"
#include "constants.h"
#include "leakdetector.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

ServerCity::ServerCity() { MVPN_COUNT_CTOR(ServerCity); }

ServerCity::ServerCity(const ServerCity& other) {
  MVPN_COUNT_CTOR(ServerCity);
  *this = other;
}

ServerCity& ServerCity::operator=(const ServerCity& other) {
  if (this == &other) return *this;

  m_name = other.m_name;
  m_code = other.m_code;
  m_latitude = other.m_latitude;
  m_longitude = other.m_longitude;
  m_servers = other.m_servers;

  return *this;
}

ServerCity::~ServerCity() { MVPN_COUNT_DTOR(ServerCity); }

bool ServerCity::fromJson(const QJsonObject& obj) {
  QJsonValue name = obj.value("name");
  if (!name.isString()) {
    return false;
  }

  QJsonValue code = obj.value("code");
  if (!code.isString()) {
    return false;
  }

  QJsonValue latitude = obj.value("latitude");
  if (!latitude.isDouble()) {
    return false;
  }

  QJsonValue longitude = obj.value("longitude");
  if (!longitude.isDouble()) {
    return false;
  }

  QJsonValue serversValue = obj.value("servers");
  if (!serversValue.isArray()) {
    return false;
  }

  QList<Server> servers;
  if (!Constants::inProduction() || !name.toString().contains("BETA")) {
    QJsonArray serversArray = serversValue.toArray();
    for (QJsonValue serverValue : serversArray) {
      if (!serverValue.isObject()) {
        return false;
      }

      QJsonObject serverObj = serverValue.toObject();

      Server server;
      if (!server.fromJson(serverObj)) {
        return false;
      }

      servers.append(server);
    }
  }

  m_name = name.toString();
  m_code = code.toString();
  m_latitude = latitude.toDouble();
  m_longitude = longitude.toDouble();
  m_servers.swap(servers);

  return true;
}
