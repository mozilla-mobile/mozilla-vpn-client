/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercity.h"
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
  m_servers = other.m_servers;

  return *this;
}

ServerCity::~ServerCity() { MVPN_COUNT_DTOR(ServerCity); }

bool ServerCity::fromJson(const QJsonObject& obj,
                          const QHash<QString, ServerExtra>& serverExtras) {
  QJsonValue name = obj.value("name");
  if (!name.isString()) {
    return false;
  }

  QJsonValue code = obj.value("code");
  if (!code.isString()) {
    return false;
  }

  QJsonValue serversValue = obj.value("servers");
  if (!serversValue.isArray()) {
    return false;
  }

  QList<Server> servers;
  QJsonArray serversArray = serversValue.toArray();
  for (QJsonValue serverValue : serversArray) {
    if (!serverValue.isObject()) {
      return false;
    }

    QJsonObject serverObj = serverValue.toObject();

    Server server;
    if (!server.fromJson(serverObj, serverExtras)) {
      return false;
    }

    servers.append(server);
  }

  m_name = name.toString();
  m_code = code.toString();
  m_servers.swap(servers);

  return true;
}
