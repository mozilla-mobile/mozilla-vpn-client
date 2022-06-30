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

 private:
  void connectionStabilityEvent();
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
  void periodicStateRecorder();
#endif

 private:
  QTimer m_connectionStabilityTimer;
#if defined(MVPN_WINDOWS) || defined(MVPN_LINUX) || defined(MVPN_MACOS)
  QTimer m_gleanControllerUpTimer;
#endif
};

#endif  // TELEMETRY_H
