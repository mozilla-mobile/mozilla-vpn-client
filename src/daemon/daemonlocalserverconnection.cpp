/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemonlocalserverconnection.h"

#if defined(MZ_MACOS)
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>

#include "daemon.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("DaemonLocalServerConnection");
}  // namespace

DaemonLocalServerConnection::DaemonLocalServerConnection(QObject* parent,
                                                         QLocalSocket* socket)
    : QObject(parent) {
  MZ_COUNT_CTOR(DaemonLocalServerConnection);

  logger.debug() << "Connection created";

  Q_ASSERT(socket);
  m_socket = socket;

  connect(m_socket, &QLocalSocket::readyRead, this,
          &DaemonLocalServerConnection::readData);

  Daemon* daemon = Daemon::instance();
  connect(daemon, &Daemon::connected, this,
          &DaemonLocalServerConnection::connected);
  connect(daemon, &Daemon::disconnected, this,
          &DaemonLocalServerConnection::disconnected);
  connect(daemon, &Daemon::backendFailure, this,
          &DaemonLocalServerConnection::backendFailure);
}

DaemonLocalServerConnection::~DaemonLocalServerConnection() {
  MZ_COUNT_DTOR(DaemonLocalServerConnection);

  logger.debug() << "Connection released";
}

void DaemonLocalServerConnection::readData() {
  logger.debug() << "Read Data";

  Q_ASSERT(m_socket);

  while (true) {
    int pos = m_buffer.indexOf("\n");
    if (pos == -1) {
      QByteArray input = m_socket->readAll();
      if (input.isEmpty()) {
        break;
      }
      m_buffer.append(input);
      continue;
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

void DaemonLocalServerConnection::parseCommand(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.error() << "Invalid input";
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue typeValue = obj.value("type");
  if (!typeValue.isString()) {
    logger.warning() << "No type command. Ignoring request.";
    return;
  }
  QString type = typeValue.toString();

  logger.debug() << "Command received:" << type;
  if (!isCallerAuthorized(type)) {
    logger.warning() << "Caller is unauthorized. Ignoring command.";
    return;
  }

  if (type == "activate") {
    InterfaceConfig config;
    if (!Daemon::parseConfig(obj, config)) {
      logger.error() << "Invalid configuration";
      emit disconnected();
      return;
    }

    if (!Daemon::instance()->activate(config)) {
      logger.error() << "Failed to activate the interface";
      emit disconnected();
    }
    return;
  }

  if (type == "deactivate") {
    Daemon::instance()->deactivate();
    return;
  }

  if (type == "status") {
    QJsonObject obj = Daemon::instance()->getStatus();
    obj.insert("type", "status");
    m_socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_socket->write("\n");
    return;
  }

  if (type == "logs") {
    QJsonObject obj;
    obj.insert("type", "logs");
    obj.insert("logs", Daemon::instance()->logs().replace("\n", "|"));
    m_socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_socket->write("\n");
    return;
  }

  if (type == "cleanlogs") {
    Daemon::instance()->cleanLogs();
    return;
  }

  logger.warning() << "Invalid command:" << type;
}

void DaemonLocalServerConnection::connected(const QString& pubkey) {
  QJsonObject obj;
  obj.insert("type", "connected");
  obj.insert("pubkey", QJsonValue(pubkey));
  write(obj);
}

void DaemonLocalServerConnection::disconnected() {
  m_sessionUid = 0;

  QJsonObject obj;
  obj.insert("type", "disconnected");
  write(obj);
}

void DaemonLocalServerConnection::backendFailure() {
  QJsonObject obj;
  obj.insert("type", "backendFailure");
  write(obj);
}

void DaemonLocalServerConnection::write(const QJsonObject& obj) {
  m_socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  m_socket->write("\n");
}

bool DaemonLocalServerConnection::isCallerAuthorized(const QString& command) {
#if defined(MZ_MACOS)
  uid_t uid;
  gid_t gid;
  int result = getpeereid(m_socket->socketDescriptor(), &uid, &gid);
  if (result == -1) {
    logger.error() << "Unable to determine if caller is authorized."
                      "Assuming unauthorized. Error:"
                   << errno;
    return false;
  }

  if (uid == 0) {
    logger.debug() << "Command issued by root.";
    // Case: This is root calling, root is always authorized.
    return true;
  }

  Daemon::State state = Daemon::instance()->state();
  if (state != Daemon::Inactive) {
    if (m_sessionUid == uid) {
      // Case: VPN is currently active and caller is the one who activated it.
      // Caller can access the APIs.
      return true;
    }

    // Case: VPN is currently active and caller is **not** the one who activated it.
    // Caller **can't** access the APIs.
    logger.error() << "Command issued by unexpected user" << uid;
    return false;
  }

  // Case: VPN is not connected and we got a request to check the status, fine.
  if (command == "status") {
    return true;
  }

  // Case: VPN is not connected and we just got an activate command.
  // Anyone can activate and we hold on to the activator's id
  // for subsequent authorizations.
  if (command == "activate") {
    logger.debug() << "New session started by user" << uid;
    m_sessionUid = uid;
    return true;
  }

  // Case: We are not connected and this is not an activate call.
  // Unless it's root, not one may access any APIs.
  logger.debug() << "Unable to determine if user" << uid
                 << "is authroized, no ongoing session.";
  return false;
#endif

  return true;
}
