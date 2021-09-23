/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTPROXYCONNECTION_H
#define ADJUSTPROXYCONNECTION_H

#include <QByteArray>
#include <QObject>
#include <QHash>

class QTcpSocket;

class AdjustProxyConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AdjustProxyConnection)

 public:
  AdjustProxyConnection(QObject* parent, QTcpSocket* connection);
  ~AdjustProxyConnection();

 private:
  void readData();

  void filterParameters();

  void forwardRequest();

 private:
  QTcpSocket* m_connection;

  QByteArray m_buffer;
  QString m_method;
  QByteArray m_route;
  QByteArray m_parametersString;
  QHash<QByteArray, QByteArray> m_headers;
  QHash<QByteArray, QByteArray> m_paramters;
};

#endif  // ADJUSTPROXYCONNECTION_H
