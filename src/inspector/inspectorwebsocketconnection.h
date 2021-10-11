/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORWEBSOCKETCONNECTION_H
#define INSPECTORWEBSOCKETCONNECTION_H

#include <QByteArray>
#include <QObject>

class QUrl;
class QQuickItem;
class QWebSocket;
class QNetworkReply;

class InspectorWebSocketConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorWebSocketConnection)

 public:
  InspectorWebSocketConnection(QObject* parent, QWebSocket* connection);
  ~InspectorWebSocketConnection();

  static void setLastUrl(const QUrl& url);
  static bool stealUrls();
  static QString appVersionForUpdate();

 private:
  void textMessageReceived(const QString& message);
  void binaryMessageReceived(const QByteArray& message);

  void parseCommand(const QByteArray& command);

  void logEntryAdded(const QByteArray& log);

  void notificationShown(const QString& title, const QString& message);

  void networkRequestFinished(QNetworkReply* reply);

  QJsonObject serialize(const QObject* target, bool requirsive);
  QString getObjectClass(const QObject* target);
 private:
  QWebSocket* m_connection;

  QByteArray m_buffer;
};

#endif  // INSPECTORWEBSOCKETCONNECTION_H
