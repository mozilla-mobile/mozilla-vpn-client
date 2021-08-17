/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSTUNNELSERVICE_H
#define WINDOWSTUNNELSERVICE_H

#include "windowstunnellogger.h"

#include <QFile>
#include <QObject>
#include <QTimer>
#include <QThread>

class WindowsTunnelService final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsTunnelService)

 public:
  WindowsTunnelService(QObject* parent = nullptr);
  ~WindowsTunnelService();

  bool start(const QString& configFile);
  void stop();
  bool isRunning();
  QString uapiCommand(const QString& command);

 signals:
  void backendFailure();

 private:
  void timeout();
  void processLogs();
  void processMessage(int index);
  int nextLogIndex();

  bool registerTunnelService(const QString& configFile);
  static QString exitCodeToFailure(unsigned int code);

 private:
  QTimer m_timer;
  QThread m_logthread;
  WindowsTunnelLogger* m_logworker = nullptr;

  // These are really SC_HANDLEs in disguise.
  void* m_scm = nullptr;
  void* m_service = nullptr;
};

#endif  // WINDOWSTUNNELSERVICE_H
