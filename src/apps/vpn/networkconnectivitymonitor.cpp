/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "networkconnectivitymonitor.h"

#include <QCoreApplication>

#include "controller.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkwatcher.h"

namespace {
Logger logger("NetworkConnectivityMonitor");
}

NetworkConnectivityMonitor::NetworkConnectivityMonitor(QObject* parent)
    : QObject(parent) {
  MZ_COUNT_CTOR(NetworkConnectivityMonitor);

  /*
   * Set up a connection to monitor when the connectioh health changes.
   * If connection health changes to NoSignal while the VPN is on, we
   * suspect there may be no network connectivity.
   * In which case we will check for the transport type, if it comes back
   * as None or Unknown then we assume the device has lost connectivity to the
   * network.
   */

  connect(
      MozillaVPN::instance()->connectionHealth(),
      &ConnectionHealth::stabilityChanged, this, []() {
        logger.debug() << "VPN connection stability has changed";
        if (MozillaVPN::instance()->controller()->state() ==
                Controller::StateOn &&
            MozillaVPN::instance()->connectionHealth()->stability() ==
                ConnectionHealth::ConnectionStability::NoSignal) {
          logger.debug() << "User has entered NoSignal while the VPN is on.";
          auto transportType =
              MozillaVPN::instance()->networkWatcher()->getCurrentTransport();
          if (transportType == "None" || transportType == "Unknown") {
            logger.debug() << "Internet probe failed after controller became "
                              "active. Device has no network connectivity.";
          }
        }
      });
}

NetworkConnectivityMonitor::~NetworkConnectivityMonitor() {
  MZ_COUNT_DTOR(NetworkConnectivityMonitor);
}

// static
NetworkConnectivityMonitor* NetworkConnectivityMonitor::instance() {
  static NetworkConnectivityMonitor* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new NetworkConnectivityMonitor(qApp);
  }
  return s_instance;
}
