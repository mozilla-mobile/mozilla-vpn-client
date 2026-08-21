/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tunnel.h"

#include <QJsonObject>

#include "logger.h"

namespace {
Logger logger("TunnelLogger");
}

bool Tunnel::supportServerSwitching(const InterfaceConfig& config) {
  if (!m_connections.contains(config.m_hopType)) {
    return false;
  }
  logger.debug() << "Checking if server switching is possible for"
                 << config.m_hopType;

  const InterfaceConfig& current =
      m_connections.value(config.m_hopType).m_config;

  return (protocolType() == config.m_protocolType &&
          current.m_privateKey == config.m_privateKey &&
          current.m_deviceIpv4Address == config.m_deviceIpv4Address &&
          current.m_deviceIpv6Address == config.m_deviceIpv6Address &&
          current.m_serverIpv4Gateway == config.m_serverIpv4Gateway &&
          current.m_serverIpv6Gateway == config.m_serverIpv6Gateway);
}

void Tunnel::removeInterfaceIfExists() {
  if (interfaceExists()) {
    if (!deleteInterface()) {
      qFatal("Interface `%s` exists and cannot be removed. Cannot proceed!",
             interfaceName().toUtf8().constData());
    }
  }
}

bool Tunnel::run(Op op, const InterfaceConfig& config) {
  logger.info() << "Running operation" << op << "for hop type"
                << config.m_hopType;
  if (op == Op::Switch || op == Op::Up) {
    m_connections[config.m_hopType] = ConnectionState(config);
    if (op == Op::Up) {
      return activate(config);
    }
  }
  if (op == Op::Down) {
    m_connections.remove(config.m_hopType);
  }
  return true;
}