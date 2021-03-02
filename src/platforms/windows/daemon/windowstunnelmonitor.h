/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSTUNNELMONITOR_H
#define WINDOWSTUNNELMONITOR_H

#include <QObject>
#include <QTimer>

class WindowsTunnelMonitor final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsTunnelMonitor)

 public:
  WindowsTunnelMonitor();
  ~WindowsTunnelMonitor();

  void start();
  void stop();

 signals:
  void backendFailure();

 private:
  void timeout();

 private:
  QTimer m_timer;
};

#endif  // WINDOWSTUNNELMONITOR_H
