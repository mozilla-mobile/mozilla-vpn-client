/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef MOZILLA_VPN_INSPECTORWEBCHANNELTRANSPORT_H
#define MOZILLA_VPN_INSPECTORWEBCHANNELTRANSPORT_H



#include <QObject>
#include <QWebChannelAbstractTransport>
#include <QWebSocket>


namespace InspectorServer {
class InspectorWebSocketTransport : public QWebChannelAbstractTransport {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorWebSocketTransport)

 public:
  ~InspectorWebSocketTransport();
  explicit InspectorWebSocketTransport(QWebSocket* connection);

  void sendMessage(const QJsonObject& message) override;

 private:
  QWebSocket* m_connection;
};

}  // namespace InspectorServer

#endif  // MOZILLA_VPN_INSPECTORWEBCHANNELTRANSPORT_H
