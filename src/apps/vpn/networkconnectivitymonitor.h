/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKCONNECTIVITYMONITOR_H
#define NETWORKCONNECTIVITYMONITOR_H

#include <QObject>

#include "connectionhealth.h"

/**
 * @brief this class monitors the Controller and ConnectionHealth to check for
 * network connectivity status after the controller enters the No Signal state.
 */
class NetworkConnectivityMonitor final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetworkConnectivityMonitor)

 public:
  static NetworkConnectivityMonitor* instance();

  ~NetworkConnectivityMonitor();

 private:
  explicit NetworkConnectivityMonitor(QObject* parent);
};

#endif  // NETWORKCONNECTIVITYMONITOR_H
