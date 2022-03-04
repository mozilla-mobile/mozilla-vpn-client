/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORWEBSOCKETCONNECTION_H
#define INSPECTORWEBSOCKETCONNECTION_H

#include "inspectorhandler.h"

class QWebSocket;

class InspectorWebSocketConnection final : public InspectorHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorWebSocketConnection)

 public:
  InspectorWebSocketConnection(QObject* parent, QWebSocket* connection);
  ~InspectorWebSocketConnection();

 private:
  void send(const QByteArray& buffer) override;

  void textMessageReceived(const QString& message);
  void binaryMessageReceived(const QByteArray& message);

 private:
  QWebSocket* m_connection = nullptr;
};

#endif  // INSPECTORWEBSOCKETCONNECTION_H
