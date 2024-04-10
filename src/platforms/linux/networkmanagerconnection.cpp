/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkmanagerconnection.h"

#include "logger.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("NetworkManagerConnection");
}

NetworkManagerConnection::NetworkManagerConnection(const QString& path,
                                                   QObject* parent)
    : QObject(parent),
      m_interface("org.freedesktop.NetworkManager", path,
                  "org.freedesktop.NetworkManager.Connection.Active",
                  QDBusConnection::systemBus(), parent) {
  MZ_COUNT_CTOR(NetworkManagerConnection);

  // Get the UUID.
  m_uuid = property("Uuid").toString();

  // Report state changes.
  QDBusConnection::systemBus().connect(
      m_interface.service(), path, m_interface.interface(), "StateChanged",
      this, SLOT(dbusStateChanged(uint, uint)));
}

NetworkManagerConnection::~NetworkManagerConnection() {
  MZ_COUNT_DTOR(NetworkManagerConnection);
}

uint NetworkManagerConnection::state() const {
  uint state = property("State").toUInt();
  logger.debug() << "fetching state" << state;
  return state;
}

void NetworkManagerConnection::dbusStateChanged(uint state, uint reason) {
  logger.debug() << "state change" << state << "because" << reason;
  emit stateChanged(state, reason);
}
