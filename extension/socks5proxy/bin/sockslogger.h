/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SOCKSLOGGER_H
#define SOCKSLOGGER_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVector>

class QDir;
class QFile;
class Socks5;
class Socks5Connection;

// Calculates a boxcar average
class BoxcarAverage final {
 public:
  BoxcarAverage(int buckets = 8) : m_buckets(buckets) { advance(); }

  // Increment the data in the current bucket.
  void addSample(qint64 sample) { m_data[0] += sample; }

  // Advance the boxcar average to the next bucket.
  void advance() {
    if (m_data.length() >= m_buckets) {
      m_data.resize(m_buckets - 1);
    }
    m_data.push_front(0);
  }

  // Calculate the average value over the buckets.
  qint64 average() const {
    qint64 sum = 0;
    for (auto x : m_data) {
      sum += x;
    }
    return m_data.isEmpty() ? 0 : sum / m_data.length();
  }

 private:
  const int m_buckets;
  QVector<qint64> m_data;
};

class SocksLogger final : public QObject {
  Q_OBJECT

 public:
  explicit SocksLogger(QObject* parent = nullptr);
  ~SocksLogger();

  static QString bytesToString(qint64 value);
  static QDebug& printEventStack(QDebug& msg, Socks5Connection* conn);
  void printStatus();

  const QString& logfile() const { return m_logFileName; }
  void setLogfile(const QString& filename);
  void setVerbose(bool enabled);

 public slots:
  void incomingConnection(Socks5Connection* conn);

 private:
  static bool makeLogDir(const QDir& dir);
  void logfileHandler(QtMsgType type, const QMessageLogContext& ctx,
                      const QString& msg);
  static void logHandler(QtMsgType type, const QMessageLogContext& ctx,
                         const QString& msg);
  void dataSentReceived(qint64 sent, qint64 received);
  void connectionStateChanged();
  void tick();

 private:
  static SocksLogger* s_instance;

  bool m_verbose = false;
  QString m_logFileName;
  QMutex m_logFileMutex;
  QFile* m_logFileDevice = nullptr;

  struct Event {
    QString m_newConnection;
    qint64 m_when;
  };
  QList<Event> m_events;
  qsizetype m_numConnections = 0;

  QTimer m_timer;
  QString m_lastStatus;

  BoxcarAverage m_rx_bytes;
  BoxcarAverage m_tx_bytes;
};

#endif  // SOCKSLOGGER_H
