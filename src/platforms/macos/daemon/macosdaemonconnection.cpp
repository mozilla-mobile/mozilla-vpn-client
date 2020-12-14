/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemonconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "macosdaemon.h"

#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger(LOG_MACOS, "MacOSDaemonConnection");
}

MacOSDaemonConnection::MacOSDaemonConnection(QObject* parent,
                                             QLocalSocket* socket)
    : QObject(parent) {
  MVPN_COUNT_CTOR(MacOSDaemonConnection);

  logger.log() << "Connection created";

  Q_ASSERT(socket);
  m_socket = socket;

  connect(m_socket, &QLocalSocket::readyRead, this,
          &MacOSDaemonConnection::readData);

  MacOSDaemon* daemon = MacOSDaemon::instance();
  connect(daemon, &MacOSDaemon::connected, this,
          &MacOSDaemonConnection::connected);
  connect(daemon, &MacOSDaemon::disconnected, this,
          &MacOSDaemonConnection::disconnected);
}

MacOSDaemonConnection::~MacOSDaemonConnection() {
  MVPN_COUNT_DTOR(MacOSDaemonConnection);

  logger.log() << "Connection released";
}

void MacOSDaemonConnection::readData() {
  logger.log() << "Read Data";

  Q_ASSERT(m_socket);
  QByteArray input = m_socket->readAll();
  m_buffer.append(input);

  while (true) {
    int pos = m_buffer.indexOf("\n");
    if (pos == -1) {
      break;
    }

    QByteArray line = m_buffer.left(pos);
    m_buffer.remove(0, pos + 1);

    QByteArray command(line);
    command = command.trimmed();

    if (command.isEmpty()) {
      continue;
    }

    parseCommand(command);
  }
}

void MacOSDaemonConnection::parseCommand(const QByteArray& data) {
  logger.log() << "Command received:" << data;

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.log() << "Invalid input";
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue typeValue = obj.value("type");
  if (!typeValue.isString()) {
    logger.log() << "No type command. Ignoring request.";
    return;
  }

  QString type = typeValue.toString();
  if (type == "activate") {
    Daemon::Config config;
    if (!MacOSDaemon::parseConfig(obj, config)) {
      logger.log() << "Invalid configuration";
      emit disconnected();
      return;
    }

    if (!MacOSDaemon::instance()->activate(config)) {
      logger.log() << "Failed to activate the interface";
      emit disconnected();
    }
    return;
  }

  if (type == "deactivate") {
    MacOSDaemon::instance()->deactivate();
    return;
  }

  if (type == "status") {
    MacOSDaemon::instance()->status(m_socket);
    return;
  }

  if (type == "logs") {
    MacOSDaemon::instance()->logs(m_socket);
    return;
  }

  if (type == "cleanlogs") {
    MacOSDaemon::instance()->cleanLogs();
    return;
  }

  logger.log() << "Invalid command:" << type;
}

void MacOSDaemonConnection::connected() {
  QJsonObject obj;
  obj.insert("type", "connected");
  write(obj);
}

void MacOSDaemonConnection::disconnected() {
  QJsonObject obj;
  obj.insert("type", "disconnected");
  write(obj);
}

void MacOSDaemonConnection::write(const QJsonObject& obj) {
  m_socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  m_socket->write("\n");
}
