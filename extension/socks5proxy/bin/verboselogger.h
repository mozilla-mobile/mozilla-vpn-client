/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VERBOSELOGGER_H
#define VERBOSELOGGER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QVector>

class Socks5;
class Socks5Connection;

// Calculates a boxcar average
class BoxcarAverage final {
 public:
  BoxcarAverage(int buckets = 8) : m_buckets(buckets) { advance(); }

  // Increment the data in the current bucket.
  void addSample(qint64 sample) {
   m_data[0] += sample;
  }

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

class VerboseLogger final : public QObject {
  Q_OBJECT

 public:
  explicit VerboseLogger(Socks5* proxy);
  ~VerboseLogger() = default;

  static QString bytesToString(qint64 value);

  void printStatus();

 private:
  static void logHandler(QtMsgType type, const QMessageLogContext& ctx,
                         const QString& msg);
  void dataSentReceived(qint64 sent, qint64 received);
  void connectionStateChanged();
  void tick();

 private:
  Socks5* m_socks = nullptr;

  struct Event {
    QString m_newConnection;
    qint64 m_when;
  };
  QList<Event> m_events;

  QTimer m_timer;
  static QString s_lastStatus;

  BoxcarAverage m_rx_bytes;
  BoxcarAverage m_tx_bytes;
};

#endif  // VERBOSELOGGER_H
