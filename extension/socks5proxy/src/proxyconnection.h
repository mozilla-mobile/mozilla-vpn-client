/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROXYCONNECTION_H
#define PROXYCONNECTION_H

#include <QByteArray>
#include <QIODevice>
#include <QLocalSocket>
#include <QObject>
#include <QTcpSocket>

class ProxyConnection : public QObject {
  Q_OBJECT

 protected:
  explicit ProxyConnection(QIODevice* socket);

 public:
  ~ProxyConnection() = default;

  quint64 sendHighWaterMark() const { return m_sendHighWaterMark; }
  quint64 recvHighWaterMark() const { return m_recvHighWaterMark; }
  const QString& errorString() const { return m_errorString; }

  static constexpr const int MAX_CONNECTION_BUFFER = 16 * 1024;

 signals:
  void setupOutSocket(qintptr sd, const QHostAddress& dest);
  void dataSentReceived(qint64 sent, qint64 received);

 protected:
  /**
   * @brief Copies incoming bytes to another QIODevice
   *
   * @param from- the source device
   * @param to- the output device
   * @param watermark- reference to the buffer high watermark
   */
  static void proxy(QIODevice* rx, QIODevice* tx, quint64& watermark);

  quint64 m_sendHighWaterMark = 0;
  quint64 m_recvHighWaterMark = 0;
  QString m_errorString;
};

#endif  // PROXYCONNECTION_H
