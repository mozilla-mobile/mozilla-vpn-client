/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORCONNECTION_H
#define INSPECTORCONNECTION_H

#include <QByteArray>
#include <QObject>

class QUrl;
class QQuickItem;
class QWebSocket;

class InspectorConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(InspectorConnection)

 public:
  InspectorConnection(QObject* parent, QWebSocket* connection);
  ~InspectorConnection();

  static void setLastUrl(const QUrl& url);

 private slots:
  void textMessageReceived(const QString& message);
  void binaryMessageReceived(const QByteArray& message);

  void parseCommand(const QByteArray& command);
  void tooManyArguments(int arguments);
  QQuickItem* findObject(const QString& name);

 private:
  QWebSocket* m_connection;

  QByteArray m_buffer;
};

#endif  // INSPECTORCONNECTION_H
