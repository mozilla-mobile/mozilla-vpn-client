/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "netmgrconnection.h"

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("NetMgrConnection");
}

NetMgrConnection::NetMgrConnection(const QString& path,
                                                   QObject* parent)
    : QObject(parent),
      m_interface("org.freedesktop.NetworkManager", path,
                  "org.freedesktop.NetworkManager.Connection.Active",
                  QDBusConnection::systemBus(), parent) {
  MZ_COUNT_CTOR(NetMgrConnection);

  // Get the UUID.
  m_uuid = property("Uuid").toString();

  // Report state changes.
  QDBusConnection::systemBus().connect(
      m_interface.service(), path, m_interface.interface(), "StateChanged",
      this, SLOT(dbusStateChanged(uint, uint)));
}

NetMgrConnection::~NetMgrConnection() {
  MZ_COUNT_DTOR(NetMgrConnection);
}

uint NetMgrConnection::state() const {
  uint state = property("State").toUInt();
  logger.debug() << "fetching state" << state;
  return state;
}

void NetMgrConnection::dbusStateChanged(uint state, uint reason) {
  logger.debug() << "state change" << state << "because" << reason;
  emit stateChanged(state, reason);
}
