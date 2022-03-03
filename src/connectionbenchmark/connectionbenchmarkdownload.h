/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARKDOWNLOAD_H
#define CONNECTIONBENCHMARKDOWNLOAD_H

#include "tasks/downloadresource/taskdownloadresource.h"

#include <initializer_list>
#include <QElapsedTimer>
#include <QList>
#include <QObject>

class ConnectionBenchmarkDownload final : public QObject {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmarkDownload);

 public:
  ConnectionBenchmarkDownload();
  ~ConnectionBenchmarkDownload();

  enum State {
    StateInitial,
    StateBenchmarking,
    StateReady,
    StateError,
  };
  State state() const { return m_state; }

  void start();
  void stop();
  double downloadSpeed() const { return m_bytesPerSecond; }

 signals:
  void stateChanged();

 private slots:
  void onReady(QByteArray data, bool hasError);

 private:
  TaskDownloadResource* m_downloadTask;
  QElapsedTimer m_timer;

  State m_state = StateInitial;
  quint64 m_bytesPerSecond;

  void setState(State state);
};

#endif  // CONNECTIONBENCHMARKDOWNLOAD_H
