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

  void startTimeToFirstScreenTimer();
  void stopTimeToFirstScreenTimer();

 private:
  void connectionStabilityEvent();
  void vpnSessionPingTimeout();

 private:
  QTimer m_connectionStabilityTimer;
  QTimer m_vpnSessionPingTimer;

  // The Glean timer id for the performance.time_to_main_screen metric.
  qint64 m_timeToFirstScreenTimerId = 0;
};

#endif  // TELEMETRY_H
