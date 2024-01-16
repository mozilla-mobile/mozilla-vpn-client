/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandhandler.h"

#include <QUrl>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace InspectorTools {

CommandHandler::CommandHandler(QObject* parent) : QObject(parent) {


}

CommandHandler::~CommandHandler() { }

const QByteArray CommandHandler::recv(const QByteArray& command) {
  qInfo() << "command received:" << command;

  if (command.isEmpty()) {
    return "";
  }

  QList<QByteArray> parts = command.split(' ');
  Q_ASSERT(!parts.isEmpty());

  QString cmdName = parts[0].trimmed();
  for (int i = 1; i < parts.length(); ++i) {
    parts[i] = QUrl::fromPercentEncoding(parts[i]).toLocal8Bit();
  }

  for (const Command& command : s_commands) {
    if (cmdName == command.m_commandName) {
      if (parts.length() != command.m_arguments + 1) {
        QJsonObject obj;
        obj["type"] = command.m_commandName;
        obj["error"] = QString("too many arguments (%1 expected)")
                           .arg(command.m_arguments);
        return QJsonDocument(obj).toJson(QJsonDocument::Compact);
      }

      QJsonObject obj = command.m_callback(parts);
      obj["type"] = command.m_commandName;
      return QJsonDocument(obj).toJson(QJsonDocument::Compact);
    }
  }

  QJsonObject obj;
  obj["type"] = "unknown";
  obj["error"] = "invalid command";
  return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

// static
void CommandHandler::registerCommand(
    const QString& commandName, const QString& commandDescription,
    int32_t arguments,
    std::function<QJsonObject(const QList<QByteArray>&)>&&
        callback) {
  registerCommand(
      Command{commandName, commandDescription, arguments,
                                     std::move(callback)}
  );
}
//static
void CommandHandler::registerCommand(const Command command){
  s_commands.append(command);
}


void CommandHandler::unregisterCommand(const QString& commandName) {
  Q_UNUSED(commandName);
  // TODO: Implement.
}

}  // namespace InspectorTools
