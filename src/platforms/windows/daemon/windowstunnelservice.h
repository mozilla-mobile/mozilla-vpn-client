/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSTUNNELSERVICE_H
#define WINDOWSTUNNELSERVICE_H

#include <QFile>
#include <QObject>
#include <QTimer>

#include "Windows.h"

class WindowsTunnelService final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsTunnelService)

 public:
  WindowsTunnelService(QObject* parent = nullptr);
  ~WindowsTunnelService();

  void resetLogs();
  bool start(const QString& configFile);
  void stop();
  bool isRunning();
  HANDLE createPipe();

 signals:
  void backendFailure();

 private:
  void timeout();
  void processLogs();
  void processMessage(int index);
  int nextLogIndex();

  bool registerTunnelService(const QString& configFile);
  bool stopAndDeleteTunnelService(SC_HANDLE service);
  static bool waitForServiceStatus(SC_HANDLE service, DWORD expectedStatus);
  static QString exitCodeToFailure(DWORD code);

 private:
  QTimer m_timer;
  QTimer m_logtimer;
  QFile* m_logfile = nullptr;
  uchar* m_logdata = nullptr;
  int m_logindex = -1;
  quint64 m_logEpochNsec = 0;

  SC_HANDLE m_scm = nullptr;
  SC_HANDLE m_service = nullptr;
};

#endif  // WINDOWSTUNNELSERVICE_H
