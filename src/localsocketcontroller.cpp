/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localsocketcontroller.h"

#include <stdint.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaType>

#include "leakdetector.h"
#include "logger.h"

// When the daemon is unreachable, we will retry indefinitely using an
// exponential backoff algorithm. The interval between retries starts at
// the initial value, and doubles after each failed connection attempt,
// with time between each retry clamped to the maximum value.
//
// This means that if the daemon never starts, the steady-state behaviour will
// have the client retry every 16 seconds indefinitely.
constexpr int CONNECTION_RETRY_INITIAL_MSEC = 500;    // 0.5 seconds
constexpr int CONNECTION_RETRY_MAXIMUM_MSEC = 16000;  // 16 seconds

namespace {
Logger logger("LocalSocketController");
}

LocalSocketController::LocalSocketController(const QString& path)
    : m_path(path) {
  MZ_COUNT_CTOR(LocalSocketController);

  m_socket = new QLocalSocket(this);
  connect(m_socket, &QLocalSocket::connected, this,
          &LocalSocketController::daemonConnected);
  connect(m_socket, &QLocalSocket::disconnected, this,
          [&] { errorOccurred(QLocalSocket::PeerClosedError); });
  connect(m_socket, &QLocalSocket::errorOccurred, this,
          &LocalSocketController::errorOccurred);
  connect(m_socket, &QLocalSocket::readyRead, this,
          &LocalSocketController::readData);

  m_initializingInterval = CONNECTION_RETRY_INITIAL_MSEC;
  m_initializingTimer.setSingleShot(true);
  connect(&m_initializingTimer, &QTimer::timeout, this,
          &LocalSocketController::initializeInternal);
}

LocalSocketController::~LocalSocketController() {
  MZ_COUNT_DTOR(LocalSocketController);
  clearAllTimeouts();
}

void LocalSocketController::errorOccurred(
    QLocalSocket::LocalSocketError error) {
  logger.error() << "Error occurred:" << error;

  if (m_daemonState != eInitializing) {
    emit backendFailure(Controller::ErrorNone);
    emit disconnected();
  }

  // We have lost communication with the daemon, try to reconnect.
  clearAllTimeouts();
  m_initializingTimer.start(m_initializingInterval);
}

void LocalSocketController::disconnectInternal() {
  // We're still eReady as the Deamon is alive
  // and can make a new connection.
  m_daemonState = eReady;
  m_initializingTimer.stop();
  emit disconnected();
}

void LocalSocketController::initialize(const Device* device, const Keys* keys) {
  logger.debug() << "Initializing";

  Q_UNUSED(device);
  Q_UNUSED(keys);

  Q_ASSERT(m_daemonState == eUnknown);

  initializeInternal();
}

void LocalSocketController::initializeInternal() {
  // Perform an exponential backoff when trying to connect to the daemon. This
  // ensures that we will reconnect gracefully even if it takes the daemon a
  // while to start up, or needs time to recover from a crash.
  if (m_daemonState != eInitializing) {
    m_initializingInterval = CONNECTION_RETRY_INITIAL_MSEC;
  } else if (m_initializingInterval < CONNECTION_RETRY_MAXIMUM_MSEC) {
    m_initializingInterval *= 2;
  }
  m_daemonState = eInitializing;

  logger.debug() << "Connecting to:" << m_path;
  m_socket->abort();
  m_socket->connectToServer(m_path);
}

void LocalSocketController::daemonConnected() {
  logger.debug() << "Daemon connected";
  Q_ASSERT(m_daemonState == eInitializing);
  checkStatus();
}

void LocalSocketController::activate(const InterfaceConfig& config,
                                     Controller::Reason reason) {
  Q_UNUSED(reason);

  QJsonObject json = config.toJson();
  json.insert("type", "activate");

  write(json);
}

void LocalSocketController::deactivate() {
  logger.debug() << "Deactivating";

  if (m_daemonState != eReady) {
    logger.debug() << "No disconnect, controller is not ready";
    emit disconnected();
    return;
  }

  QJsonObject json;
  json.insert("type", "deactivate");
  write(json);
}

void LocalSocketController::checkStatus() {
  logger.debug() << "Check status";

  if (m_daemonState == eReady || m_daemonState == eInitializing) {
    Q_ASSERT(m_socket);

    QJsonObject json;
    json.insert("type", "status");
    write(json, "status");
  }
}

void LocalSocketController::getBackendLogs(QIODevice* device) {
  logger.debug() << "Backend logs";

  if (m_logReceiver) {
    m_logReceiver->close();
    m_logReceiver = nullptr;
  }

  if (m_daemonState != eReady) {
    device->close();
    return;
  }

  QJsonObject json;
  json.insert("type", "logs");
  write(json);

  m_logReceiver = device;
}

void LocalSocketController::cleanupBackendLogs() {
  logger.debug() << "Cleanup logs";

  if (m_daemonState != eReady) {
    return;
  }

  QJsonObject json;
  json.insert("type", "cleanlogs");
  write(json, "logs", 5000);
}

void LocalSocketController::readData() {
  logger.debug() << "Reading";

  Q_ASSERT(m_socket);
  Q_ASSERT(m_daemonState == eInitializing || m_daemonState == eReady);
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

void LocalSocketController::parseCommand(const QByteArray& command) {
  QJsonDocument json = QJsonDocument::fromJson(command);
  if (!json.isObject()) {
    logger.error() << "Invalid JSON - object expected";
    return;
  }

  QJsonObject obj = json.object();
  QJsonValue typeValue = obj.value("type");
  if (!typeValue.isString()) {
    logger.error() << "Invalid JSON - no type";
    return;
  }
  QString type = typeValue.toString();

  logger.debug() << "Parse command:" << type;
  clearTimeout(type);

  if (m_daemonState == eInitializing && type == "status") {
    m_daemonState = eReady;

    QJsonValue connected = obj.value("connected");
    if (!connected.isBool()) {
      logger.error() << "Invalid JSON for status - connected expected";
      return;
    }

    QDateTime datetime;
    if (connected.toBool()) {
      QJsonValue date = obj.value("date");
      if (!date.isString()) {
        logger.error() << "Invalid JSON for status - date expected";
        return;
      }

      datetime = QDateTime::fromString(date.toString());
      if (!datetime.isValid()) {
        logger.error() << "Invalid JSON for status - date is invalid";
        return;
      }
    }

    emit initialized(true, connected.toBool(), datetime);
    return;
  }

  if (m_daemonState != eReady) {
    logger.error() << "Unexpected command";
    return;
  }

  if (type == "status") {
    emitStatusFromJson(obj);
    return;
  }

  if (type == "disconnected") {
    disconnectInternal();
    return;
  }

  if (type == "connected") {
    QJsonValue pubkey = obj.value("pubkey");
    if (!pubkey.isString()) {
      logger.error() << "Unexpected pubkey value";
      return;
    }

    logger.debug() << "Handshake completed with:"
                   << logger.keys(pubkey.toString());
    emit connected(pubkey.toString());
    return;
  }

  if (type == "backendFailure") {
    int errorCode = obj.value("errorCode").toInt(Controller::ErrorNone);
    emit backendFailure(static_cast<Controller::ErrorCode>(errorCode));
    return;
  }

  if (type == "logs") {
    // We don't care if we are not waiting for logs.
    if (!m_logReceiver) {
      return;
    }

    QJsonValue logs = obj.value("logs");
    QString logString;
    if (logs.isString()) {
      logString = logs.toString().replace("|", "\n");
    }

    m_logReceiver->write(logString.toUtf8());
    m_logReceiver->close();
    m_logReceiver = nullptr;
    return;
  }

  logger.warning() << "Invalid command received:" << command;
}

void LocalSocketController::write(const QJsonObject& message,
                                  const QString& expectedResponseType,
                                  int timeout) {
  QByteArray payload = QJsonDocument(message).toJson(QJsonDocument::Compact);
  payload.append('\n');

  // If an immediate response to this message is expected, start a timer to
  // throw an error if that response fails to arrive in a timely manner. This
  // is used to detect a crash or failure of the daemon.
  if (!expectedResponseType.isEmpty()) {
    QTimer* t = new QTimer(this);
    connect(t, &QTimer::timeout, this,
            [&] { this->errorOccurred(QLocalSocket::SocketTimeoutError); });

    t->setProperty("responseType", QVariant(expectedResponseType));
    t->setSingleShot(true);
    t->start(timeout);

    m_responseTimeouts.append(t);
  }

  Q_ASSERT(m_socket);
  m_socket->write(payload);
  m_socket->flush();
}

void LocalSocketController::clearTimeout(const QString& responseType) {
  // We assume that responses arrive in the same order, the local socket
  // API has no mechanism to detect out-of-order responses, and any change
  // here to add such a mechanism would risk a compatibility issue.
  for (QTimer* t : m_responseTimeouts) {
    QVariant timerResponseType = t->property("responseType");
    if (timerResponseType.typeId() != QMetaType::QString) {
      continue;
    }
    if (timerResponseType.toString() != responseType) {
      continue;
    }
    m_responseTimeouts.removeOne(t);
    delete t;
  }
}

void LocalSocketController::clearAllTimeouts() {
  while (!m_responseTimeouts.isEmpty()) {
    QTimer* t = m_responseTimeouts.takeFirst();
    delete t;
  }
}
