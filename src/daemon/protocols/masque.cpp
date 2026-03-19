/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "masque.h"
#include <QJsonObject>
#include "logger.h"

namespace {
Logger logger("MasqueTunnel");
}

QJsonObject MasqueTunnel::getStatus() const {
  QJsonObject json;
  json.insert("connected", QJsonValue(true));
  const ConnectionState& connection = m_connections.first();
  json.insert("serverIpv4Gateway",
              QJsonValue(connection.m_config.m_serverIpv4Gateway));
  json.insert("deviceIpv4Address",
              QJsonValue(connection.m_config.m_deviceIpv4Address));
  json.insert("date", connection.m_date.toString());
  json.insert("txBytes", QJsonValue(0)); // mock values for now
  json.insert("rxBytes", QJsonValue(0)); // mock values for now
  return json;
}

int MasqueTunnel::checkHandshake() {
  // Just check if we are connected, maybe rename this function to checkConnection
  const ConnectionState& connection = m_connections.first();
  if (!connection.m_date.isValid()) {
    m_connections[connection.m_config.m_hopType].m_date =
        QDateTime::currentDateTime();
    if(interfaceExists()) {
      emit connected(connection.m_config.m_serverPublicKey);
      return 0;
    }
  }
  return 1;
}

bool MasqueTunnel::activate(const InterfaceConfig& config) {
  logger.debug() << "Activating Masque interface.";
  return addInterface(config);
}

bool MasqueTunnel::switchServer(const InterfaceConfig& config) {
  logger.debug() << "Switching Masque server.";
  if (!deleteInterface()) {
    logger.error() << "Failed to delete existing interface during server switch.";
    return false;
  }
  return addInterface(config);
}