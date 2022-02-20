/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARK_H
#define CONNECTIONBENCHMARK_H

#include "filedownloader.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

class ConnectionBenchmark final : public QObject {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmark);

  Q_PROPERTY(State state READ state NOTIFY stateChanged);
  Q_PROPERTY(
      float downloadSpeed READ downloadSpeed NOTIFY downloadSpeedChanged);

 public:
  enum State {
    StateInitial,
    StateTesting,
    StateReady,
    StateError,
  };
  Q_ENUM(State);

 public:
  ConnectionBenchmark();
  ~ConnectionBenchmark();

  State state() const { return m_state; }

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();

  void downloadFiles();

  double downloadSpeed() const { return m_mBitsPerSecond; }

 signals:
  void stateChanged();
  void downloadSpeedChanged();

 private slots:
  void onDownloaded(FileDownloader* downloader);

 private:
  State m_state = StateInitial;

  QList<FileDownloader*> m_fileDownloaderList;

  quint64 m_startTime;
  float m_mBitsPerSecond;

  QStringList m_downloadUrls;
  int m_numOfFilesTotal;
  int m_numOfFilesReceived;

  void setState(State state);

  void populateDownloadUrls();
};

#endif  // CONNECTIONBENCHMARK_H
