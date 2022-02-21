/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARKDOWNLOAD_H
#define CONNECTIONBENCHMARKDOWNLOAD_H

#include "filedownloader.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>

class ConnectionBenchmarkDownload final : public QObject {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmarkDownload);

  Q_PROPERTY(State state READ state NOTIFY stateChanged);
  Q_PROPERTY(
      double downloadSpeed READ downloadSpeed NOTIFY downloadSpeedChanged);

 public:
  ConnectionBenchmarkDownload();
  ~ConnectionBenchmarkDownload();

  enum State {
    StateInitial,
    StateTesting,
    StateReady,
    StateError,
  };
  Q_ENUM(State);

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();

  State state() const { return m_state; }
  double downloadSpeed() const { return m_bytesPerSecond; }

 signals:
  void stateChanged();
  void downloadSpeedChanged();

 private slots:
  void onReady(FileDownloader* downloader);

 private:
  State m_state = StateInitial;

  QTimer* m_timer;

  QList<FileDownloader*> m_fileDownloaderList;
  quint64 m_startTime;
  quint64 m_bytesPerSecond;

  QStringList m_downloadUrls;
  int m_numOfFilesTotal;
  int m_numOfFilesReceived;

  void populateUrlList();
  void setState(State state);
};

#endif  // CONNECTIONBENCHMARKDOWNLOAD_H
