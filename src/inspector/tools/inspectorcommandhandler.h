/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVCMD_HANDLER_H
#define DEVCMD_HANDLER_H

#include <QByteArray>
#include <QObject>

class QNetworkReply;
class QUrl;
class QQuickItem;

namespace InspectorTools {

class DevCmdHandler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DevCmdHandler)

 public:
  explicit DevCmdHandler(QObject* parent);
  ~DevCmdHandler();

  static QString getObjectClass(const QObject* target);
  static QJsonObject getViewTree();
  static QJsonObject serialize(QQuickItem* item);
  static void itemsPicked(const QList<QQuickItem*>& objects);

  /**
  * Must be called when there is an incoming command. 
  */
  Q_INVOKABLE void recv(const QByteArray& buffer);

  void send(const QByteArray& buffer) { emit onSend(buffer);}
  /**
  * Emitted when there is outGoing Data.
  */
  Q_SIGNAL void onSend(const QByteArray& buffer);

  /**
   * @brief Register a new command.
   */
  static void registerCommand(
      const QString& commandName, const QString& commandDescription,
      int32_t arguments,
      std::function<QJsonObject(DevCmdHandler*, const QList<QByteArray>&)>&&
          callback);
  // Unregister a Command
  static void unregisterCommand(const QString& commandName);


 private:
  void addonLoadCompleted();
  void logEntryAdded(const QByteArray& log);
  void networkRequestFinished(QNetworkReply* reply);
};
}  // namespace InspectorTools
#endif  // DEVCMD_HANDLER_H
