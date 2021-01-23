/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsdaemonconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "windowsdaemon.h"

#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
Logger logger(LOG_WINDOWS, "WindowsDaemonConnection");
}

WindowsDaemonConnection::WindowsDaemonConnection(QObject* parent,
                                                 QLocalSocket* socket)
    : QObject(parent) {
  MVPN_COUNT_CTOR(WindowsDaemonConnection);

  logger.log() << "Connection created";

  Q_ASSERT(socket);
  m_socket = socket;

  connect(m_socket, &QLocalSocket::readyRead, this,
          &WindowsDaemonConnection::readData);

  WindowsDaemon* daemon = WindowsDaemon::instance();
  connect(daemon, &WindowsDaemon::connected, this,
          &WindowsDaemonConnection::connected);
  connect(daemon, &WindowsDaemon::disconnected, this,
          &WindowsDaemonConnection::disconnected);
}

WindowsDaemonConnection::~WindowsDaemonConnection() {
  MVPN_COUNT_DTOR(WindowsDaemonConnection);

  logger.log() << "Connection released";
}

void WindowsDaemonConnection::readData() {
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

void WindowsDaemonConnection::parseCommand(const QByteArray& data) {
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
    if (!WindowsDaemon::parseConfig(obj, config)) {
      logger.log() << "Invalid configuration";
      emit disconnected();
      return;
    }

    if (!WindowsDaemon::instance()->activate(config)) {
      logger.log() << "Failed to activate the interface";
      emit disconnected();
    }
    return;
  }

  if (type == "deactivate") {
    WindowsDaemon::instance()->deactivate();
    return;
  }

  if (type == "status") {
    WindowsDaemon::instance()->status(m_socket);
    return;
  }

  if (type == "logs") {
    WindowsDaemon::instance()->logs(m_socket);
    return;
  }

  if (type == "cleanlogs") {
    WindowsDaemon::instance()->cleanLogs();
    return;
  }

  logger.log() << "Invalid command:" << type;
}

void WindowsDaemonConnection::connected() {
  QJsonObject obj;
  obj.insert("type", "connected");
  write(obj);
}

void WindowsDaemonConnection::disconnected() {
  QJsonObject obj;
  obj.insert("type", "disconnected");
  write(obj);
}

void WindowsDaemonConnection::write(const QJsonObject& obj) {
  m_socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  m_socket->write("\n");
}
