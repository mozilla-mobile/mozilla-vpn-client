//
// Created by Basti on 14/07/2023.
//

#include "inspectorwebsockettransport.h"

#include <QJsonDocument>
#include <QJsonObject>

InspectorWebSocketTransport::InspectorWebSocketTransport(QWebSocket* connection)
    : QWebChannelAbstractTransport(connection) {
  m_connection = connection;

  connect(connection, &QWebSocket::textMessageReceived, this,
          [this](const QString& message) {
            QJsonParseError jsonError;
            QJsonDocument json =
                QJsonDocument::fromJson(message.toLocal8Bit(), &jsonError);
            if (QJsonParseError::NoError != jsonError.error) {
              return;
            }
            if (!json.isObject()) {
              return;
            }
            emit messageReceived(json.object(), this);
          });

  connect(connection, &QWebSocket::binaryMessageReceived, this,
          [this](const QByteArray& message) {
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(message, &jsonError);
            if (QJsonParseError::NoError != jsonError.error) {
              return;
            }
            if (!json.isObject()) {
              return;
            }
            emit messageReceived(json.object(), this);
          });
}
InspectorWebSocketTransport::~InspectorWebSocketTransport() {}

void InspectorWebSocketTransport::sendMessage(const QJsonObject& message) {
  QJsonDocument json;
  json.setObject(message);
  auto json_bytes = json.toJson(QJsonDocument::JsonFormat::Compact);
  if (m_connection == nullptr) {
    return;
  }
  m_connection->sendTextMessage(json_bytes);
}
