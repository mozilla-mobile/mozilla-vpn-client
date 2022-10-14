/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONBENCHMARK_H
#define CONNECTIONBENCHMARK_H

#include "benchmarktask.h"
#include "constants.h"

#include <QList>
#include <QObject>
#include <QUrl>

class ConnectionHealth;

class ConnectionBenchmark final : public QObject {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(ConnectionBenchmark);

  Q_PROPERTY(QString downloadUrl READ downloadUrl WRITE setDownloadUrl NOTIFY
                 downloadUrlChanged)
  Q_PROPERTY(QString uploadUrl READ uploadUrl WRITE setUploadUrl NOTIFY
                 uploadUrlChanged)
  Q_PROPERTY(State state READ state NOTIFY stateChanged);
  Q_PROPERTY(Speed speed READ speed NOTIFY speedChanged);
  Q_PROPERTY(quint64 downloadBps READ downloadBps NOTIFY downloadBpsChanged);
  Q_PROPERTY(quint16 pingLatency READ pingLatency NOTIFY pingLatencyChanged);
  Q_PROPERTY(quint64 uploadBps READ uploadBps NOTIFY uploadBpsChanged);

 public:
  ConnectionBenchmark();
  ~ConnectionBenchmark();

  void initialize();

  Q_INVOKABLE void start();
  Q_INVOKABLE void reset();

  enum State {
    StateInitial,
    StateRunning,
    StateReady,
    StateError,
  };
  Q_ENUM(State);

  enum Speed {
    SpeedSlow,
    SpeedMedium,
    SpeedFast,
  };
  Q_ENUM(Speed);

  State state() const { return m_state; }
  Speed speed() const { return m_speed; }
  quint16 pingLatency() const { return m_pingLatency; }
  quint64 downloadBps() const { return m_downloadBps; }
  quint64 uploadBps() const { return m_uploadBps; }

  QString downloadUrl() const { return m_downloadUrl.toString(); }
  void setDownloadUrl(QString url) {
    m_downloadUrl.setUrl(url);
    emit downloadUrlChanged();
  }

  QString uploadUrl() const { return m_uploadUrl.toString(); }
  void setUploadUrl(QString url) {
    m_uploadUrl.setUrl(url);
    emit uploadUrlChanged();
  }

 signals:
  void downloadBpsChanged();
  void pingLatencyChanged();
  void uploadBpsChanged();
  void speedChanged();
  void stateChanged();
  void downloadUrlChanged();
  void uploadUrlChanged();

 private:
  void downloadBenchmarked(quint64 bitsPerSec, bool hasUnexpectedError);
  void pingBenchmarked(quint64 pingLatencyLatency);
  void uploadBenchmarked(quint64 bitsPerSec, bool hasUnexpectedError);

  void handleControllerState();
  void handleStabilityChange();
  void setConnectionSpeed();
  void setState(State state);
  void stop();

 private:
  QUrl m_downloadUrl = QUrl(Constants::BENCHMARK_DOWNLOAD_URL);
  QUrl m_uploadUrl = QUrl(Constants::BENCHMARK_UPLOAD_URL);

  QList<BenchmarkTask*> m_benchmarkTasks;

  State m_state = StateInitial;
  Speed m_speed = SpeedSlow;

  quint64 m_downloadBps = 0;
  quint16 m_pingLatency = 0;
  quint64 m_uploadBps = 0;
};

#endif  // CONNECTIONBENCHMARK_H
