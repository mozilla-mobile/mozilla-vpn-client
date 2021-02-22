/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORHTTPCONNECTION_H
#define INSPECTORHTTPCONNECTION_H

#include <QByteArray>
#include <QObject>

class QTcpSocket;

class InspectorHttpConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorHttpConnection)

 public:
  InspectorHttpConnection(QObject* parent, QTcpSocket* connection);
  ~InspectorHttpConnection();

 private:
  void readData();

  void processHeaders();

 private:
  QTcpSocket* m_connection;

  QByteArray m_buffer;
  QStringList m_headers;
};

#endif  // INSPECTORHTTPCONNECTION_H
