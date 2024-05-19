/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <QObject>
#include <QTimer>

class Telemetry final : public QObject {
 public:
  Telemetry();
  ~Telemetry();

  void initialize();

  static void startTimeToFirstScreenTimer();
  static void stopTimeToFirstScreenTimer();

 private:
  void connectionStabilityEvent();
  void vpnSessionPingTimeout();

 private:
  QTimer m_connectionStabilityTimer;
  QTimer m_vpnSessionPingTimer;
};

#endif  // TELEMETRY_H
