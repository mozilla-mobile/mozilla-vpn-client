/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkdownload.h"
#include "constants.h"
#include "filedownloader.h"
#include "leakdetector.h"
#include "logger.h"
#include "timersingleshot.h"

#include <QDateTime>
#include <QUrl>
#include <math.h>

namespace {
Logger logger(LOG_MODEL, "ConnectionBenchmarkDownload");
}

ConnectionBenchmarkDownload::ConnectionBenchmarkDownload() {
  MVPN_COUNT_CTOR(ConnectionBenchmarkDownload);
}

ConnectionBenchmarkDownload::~ConnectionBenchmarkDownload() {
  MVPN_COUNT_DTOR(ConnectionBenchmarkDownload);
}

void ConnectionBenchmarkDownload::populateUrlList() {
  logger.debug() << "Add download urls";

  m_downloadUrls
      << "https://speed1.syseleven.net.prod.hosts.ooklaserver.net:8080/"
         "download?nocache=73d775b0-3082-47fb-8816-d6171c023fa2&size=25000000";
  m_downloadUrls
      << "https://warp.cronon.net:8080/"
         "download?nocache=0e85d3af-efdf-491b-8b26-e8c2d0ffcbb6&size=25000000";
  m_downloadUrls
      << "https://ber.wsqm.telekom-dienste.de.prod.hosts.ooklaserver.net:8080/"
         "download?nocache=c97cb86a-a793-44a6-b933-a824334411e0&size=25000000";
  m_downloadUrls
      << "https://speedtest.sewan-group.de.prod.hosts.ooklaserver.net:8080/"
         "download?nocache=ee336c0e-a0bc-4083-aaf9-6503790febb3&size=25000000";
  m_downloadUrls
      << "https://speed1.syseleven.net.prod.hosts.ooklaserver.net:8080/"
         "download?nocache=4519563d-26d6-4e69-bf7d-3b3334369013&size=25000000";
  m_downloadUrls
      << "https://warp.cronon.net:8080/"
         "download?nocache=2fee0741-8666-4b1d-be7d-407cf1940253&size=25000000";
  m_downloadUrls
      << "https://ber.wsqm.telekom-dienste.de.prod.hosts.ooklaserver.net:8080/"
         "download?nocache=67b1e110-3b15-4877-87bc-6964f65a2af0&size=25000000";
  m_downloadUrls
      << "https://speedtest.sewan-group.de.prod.hosts.ooklaserver.net:8080/"
         "download?nocache=993866ce-83d7-4ea5-bca0-ac0629eace05&size=25000000";
  m_downloadUrls
      << "https://speed1.syseleven.net.prod.hosts.ooklaserver.net:8080/"
         "download?nocache=baf55ab4-0f5e-46ea-b6c8-f6733d546f5f&size=25000000";
  m_downloadUrls
      << "https://warp.cronon.net:8080/"
         "download?nocache=a6ce7368-6819-461d-97a5-c995b1de7ebc&size=25000000";

  m_numOfFilesTotal = m_downloadUrls.size();
}

void ConnectionBenchmarkDownload::setState(State state) {
  logger.debug() << "Set state" << state;
  m_state = state;

  emit stateChanged();
}

void ConnectionBenchmarkDownload::onReady(FileDownloader* downloader) {
  logger.debug() << "On downloaded";

  quint64 downloadDuration = QDateTime::currentMSecsSinceEpoch() - m_startTime;
  quint64 bytesPerSecond =
      downloader->bytesReceived() / downloadDuration * 1000;
  // double bitsPerSecond = bytesPerSecond * 8;
  // double mBitsPerSecond = bitsPerSecond / pow(1024, 2);

  m_bytesPerSecond += bytesPerSecond;
  m_numOfFilesReceived += 1;

  if (m_numOfFilesReceived == m_numOfFilesTotal) {
    m_fileDownloaderList.clear();

    logger.debug() << "Download speed" << m_bytesPerSecond;
    emit downloadSpeedChanged();
    setState(StateReady);
  }
}

void ConnectionBenchmarkDownload::start() {
  logger.debug() << "Start benchmark";

  ConnectionBenchmarkDownload::populateUrlList();

  // Stop speedtest when max runtime is reached
  m_timer = new QTimer();
  connect(m_timer, &QTimer::timeout, this, &ConnectionBenchmarkDownload::stop);
  m_timer->start(Constants::CONNECTION_SPEED_BENCHMARK_DURATION);

  m_startTime = QDateTime::currentMSecsSinceEpoch();
  m_bytesPerSecond = 0;
  m_numOfFilesReceived = 0;

  setState(StateTesting);

  for (const QString& downloadUrl : m_downloadUrls) {
    FileDownloader* downloader = new FileDownloader(downloadUrl, this);
    m_fileDownloaderList.append(downloader);

    connect(downloader, &FileDownloader::downloaded, this, [this, downloader] {
      ConnectionBenchmarkDownload::onReady(downloader);
    });
    connect(downloader, &FileDownloader::aborted, this, [this, downloader] {
      ConnectionBenchmarkDownload::onReady(downloader);
    });
  }

  m_downloadUrls.clear();
}

void ConnectionBenchmarkDownload::stop() {
  logger.debug() << "Stop benchmark";

  if (m_timer->isActive()) {
    m_timer->stop();
  }

  for (FileDownloader* downloader : m_fileDownloaderList) {
    downloader->abort();
  }
}
