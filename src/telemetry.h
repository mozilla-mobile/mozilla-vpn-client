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
#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS)
  void periodicStateRecorder();
#endif

#if defined(MZ_ANDROID)
  /**
   * @brief (Slot) - Processes a Daemon-Init response and records the daemon
   * status to glean.
   *
   * @param data - A JSON string containting the response of EVENT_INIT
   * @return * void
   */
  void onDaemonStatus(const QString& data);
#endif

 private:
  QTimer m_connectionStabilityTimer;
  QTimer m_vpnSessionPingTimer;
#if defined(MZ_WINDOWS) || defined(MZ_LINUX) || defined(MZ_MACOS)
  QTimer m_gleanControllerUpTimer;
#endif

  // The Glean timer id for the performance.time_to_main_screen metric.
  qint64 m_timeToFirstScreenTimerId = 0;
};

#endif  // TELEMETRY_H
