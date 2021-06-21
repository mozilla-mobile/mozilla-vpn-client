/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSTUNNELMONITOR_H
#define WINDOWSTUNNELMONITOR_H

#include <QFile>
#include <QObject>
#include <QTimer>

class WindowsTunnelMonitor final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsTunnelMonitor)

 public:
  WindowsTunnelMonitor();
  ~WindowsTunnelMonitor();

  void resetLogs();
  void start();
  void stop();

 signals:
  void backendFailure();

 private:
  void timeout();
  void processLogs();
  void processMessage(int index);
  int nextLogIndex();

 private:
  QTimer m_timer;
  QTimer m_logtimer;
  QFile* m_logfile = nullptr;
  uchar* m_logdata = nullptr;
  int m_logindex = -1;
  quint64 m_logEpochNsec = 0;
};

#endif  // WINDOWSTUNNELMONITOR_H
