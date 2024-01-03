/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DEVCMD_HANDLER_H
#define DEVCMD_HANDLER_H

#include <QByteArray>
#include <QObject>

#include "./command.h"


class QNetworkReply;
class QUrl;
class QQuickItem;

namespace InspectorTools {

class CommandHandler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(CommandHandler)

 public:
  explicit CommandHandler(QObject* parent);
  ~CommandHandler();

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
  void registerCommand(
      const QString& commandName, const QString& commandDescription,
      int32_t arguments,
      std::function<QJsonObject(const QList<QByteArray>&)>&&
          callback);
  /**
   * @brief Register a new command.
   */
  void registerCommand(const Command& command);

  // Unregister a Command
  void unregisterCommand(const QString& commandName);


 private:
   QList<Command> s_commands ;
};
}  // namespace InspectorTools
#endif  // DEVCMD_HANDLER_H
