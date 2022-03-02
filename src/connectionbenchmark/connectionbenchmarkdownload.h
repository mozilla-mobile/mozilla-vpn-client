/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARKDOWNLOAD_H
#define CONNECTIONBENCHMARKDOWNLOAD_H

#include "resourcedownloader.h"

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QTimer>

class ConnectionBenchmarkDownload final : public QObject {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmarkDownload);

 public:
  ConnectionBenchmarkDownload();
  ~ConnectionBenchmarkDownload();

  enum State {
    StateInitial,
    StateDownloading,
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
  void onReady(ResourceDownloader* downloader);

 private:
  State m_state = StateInitial;

  QTimer* m_timer = new QTimer();
  quint64 m_startTime;
  quint64 m_bytesPerSecond;

  QStringList m_downloadUrls;
  QList<ResourceDownloader*> m_resourceDownloaderList;
  int m_numOfFilesTotal;
  int m_numOfFilesReceived;

  void setState(State state);
  void populateUrlList();
};

#endif  // CONNECTIONBENCHMARKDOWNLOAD_H
