/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "localsocketcontroller.h"
#include "errorhandler.h"
#include "ipaddressrange.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/server.h"
#include "mozillavpn.h"
#include "settingsholder.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>
#include <QHostAddress>

namespace {
Logger logger(LOG_CONTROLLER, "LocalSocketController");
}

LocalSocketController::LocalSocketController() {
  MVPN_COUNT_CTOR(LocalSocketController);

  m_socket = new QLocalSocket(this);
  connect(m_socket, &QLocalSocket::connected, this,
          &LocalSocketController::daemonConnected);
  connect(m_socket, &QLocalSocket::disconnected, this,
          &LocalSocketController::disconnected);
  connect(m_socket, &QLocalSocket::errorOccurred, this,
          &LocalSocketController::errorOccurred);
  connect(m_socket, &QLocalSocket::readyRead, this,
          &LocalSocketController::readData);
}

LocalSocketController::~LocalSocketController() {
  MVPN_COUNT_DTOR(LocalSocketController);
}

void LocalSocketController::errorOccurred(
    QLocalSocket::LocalSocketError error) {
  logger.error() << "Error occurred:" << error;

  if (m_state == eInitializing) {
    emit initialized(false, false, QDateTime());
  }

  m_state = eDisconnected;
  MozillaVPN::instance()->errorHandle(ErrorHandler::ControllerError);
  emit disconnected();
}

void LocalSocketController::initialize(const Device* device, const Keys* keys) {
  logger.debug() << "Initializing";

  Q_UNUSED(device);
  Q_UNUSED(keys);

  Q_ASSERT(m_state == eUnknown);
  m_state = eInitializing;

#ifdef MVPN_WINDOWS
  QString path = "\\\\.\\pipe\\mozillavpn";
#else
  QString path = "/var/run/mozillavpn/daemon.socket";
  if (!QFileInfo::exists(path)) {
    path = "/tmp/mozillavpn.socket";
  }
#endif

  logger.debug() << "Connecting to:" << path;
  m_socket->connectToServer(path);
}

void LocalSocketController::daemonConnected() {
  logger.debug() << "Daemon connected";
  Q_ASSERT(m_state == eInitializing);
  checkStatus();
}

void LocalSocketController::activate(
    const QList<Server>& serverList, const Device* device, const Keys* keys,
    const QList<IPAddressRange>& allowedIPAddressRanges,
    const QList<QString>& vpnDisabledApps, const QHostAddress& dnsServer,
    Reason reason) {
  Q_UNUSED(reason);
  Q_ASSERT(!serverList.isEmpty());

  // Clear out any connections that might have been lingering.
  m_activationQueue.clear();
  m_device = device;
  m_keys = keys;

  if (m_state != eReady) {
    emit disconnected();
    return;
  }

  // Activate connections starting from the outermost tunnel
  for (int hopindex = serverList.count() - 1; hopindex > 0; hopindex--) {
    const Server& next = serverList[hopindex - 1];
    HopConnection hop;
    hop.m_server = serverList[hopindex];
    hop.m_hopindex = hopindex;
    hop.m_allowedIPAddressRanges.append(IPAddressRange(next.ipv4AddrIn()));
    hop.m_allowedIPAddressRanges.append(IPAddressRange(next.ipv6AddrIn()));
    m_activationQueue.append(hop);
  }

  // The final hop should be activated last.
  HopConnection lastHop;
  lastHop.m_server = serverList.first();
  lastHop.m_hopindex = 0;
  lastHop.m_allowedIPAddressRanges = allowedIPAddressRanges;
  lastHop.m_vpnDisabledApps = vpnDisabledApps;
  lastHop.m_dnsServer = dnsServer;
  m_activationQueue.append(lastHop);

  logger.debug() << "Activating";
  activateNext();
}

void LocalSocketController::activateNext() {
  const HopConnection& hop = m_activationQueue.first();
  QJsonObject json;
  json.insert("type", "activate");
  json.insert("hopindex", QJsonValue((double)hop.m_hopindex));
  json.insert("privateKey", QJsonValue(m_keys->privateKey()));
  json.insert("deviceIpv4Address", QJsonValue(m_device->ipv4Address()));
  json.insert("deviceIpv6Address", QJsonValue(m_device->ipv6Address()));
  json.insert("serverPublicKey", QJsonValue(hop.m_server.publicKey()));
  json.insert("serverIpv4AddrIn", QJsonValue(hop.m_server.ipv4AddrIn()));
  json.insert("serverIpv6AddrIn", QJsonValue(hop.m_server.ipv6AddrIn()));
  json.insert("serverPort", QJsonValue((double)hop.m_server.choosePort()));
  if (hop.m_hopindex == 0) {
    json.insert("serverIpv4Gateway", QJsonValue(hop.m_server.ipv4Gateway()));
    json.insert("serverIpv6Gateway", QJsonValue(hop.m_server.ipv6Gateway()));
    json.insert("dnsServer", QJsonValue(hop.m_dnsServer.toString()));
  }

  QJsonArray allowedIPAddesses;
  for (const IPAddressRange& i : hop.m_allowedIPAddressRanges) {
    QJsonObject range;
    range.insert("address", QJsonValue(i.ipAddress()));
    range.insert("range", QJsonValue((double)i.range()));
    range.insert("isIpv6", QJsonValue(i.type() == IPAddressRange::IPv6));
    allowedIPAddesses.append(range);
  };
  json.insert("allowedIPAddressRanges", allowedIPAddesses);
  QJsonArray splitTunnelApps;
  for (const auto& uri : hop.m_vpnDisabledApps) {
    splitTunnelApps.append(QJsonValue(uri));
  }
  json.insert("vpnDisabledApps", splitTunnelApps);

  write(json);
}

void LocalSocketController::deactivate(Reason reason) {
  logger.debug() << "Deactivating";

  m_activationQueue.clear();

  if (m_state != eReady) {
    emit disconnected();
    return;
  }

  if (reason == ReasonSwitching) {
    logger.debug() << "No disconnect for quick server switching";
    emit disconnected();
    return;
  }

  QJsonObject json;
  json.insert("type", "deactivate");
  write(json);
}

void LocalSocketController::checkStatus() {
  logger.debug() << "Check status";

  if (m_state == eReady || m_state == eInitializing) {
    Q_ASSERT(m_socket);

    QJsonObject json;
    json.insert("type", "status");
    write(json);
  }
}

void LocalSocketController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  logger.debug() << "Backend logs";

  if (m_logCallback) {
    m_logCallback("");
    m_logCallback = nullptr;
  }

  if (m_state != eReady) {
    std::function<void(const QString&)> callback = a_callback;
    callback("");
    return;
  }

  m_logCallback = std::move(a_callback);

  QJsonObject json;
  json.insert("type", "logs");
  write(json);
}

void LocalSocketController::cleanupBackendLogs() {
  logger.debug() << "Cleanup logs";

  if (m_logCallback) {
    m_logCallback("");
    m_logCallback = nullptr;
  }

  if (m_state != eReady) {
    return;
  }

  QJsonObject json;
  json.insert("type", "cleanlogs");
  write(json);
}

void LocalSocketController::readData() {
  logger.debug() << "Reading";

  Q_ASSERT(m_socket);
  Q_ASSERT(m_state == eInitializing || m_state == eReady);
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
  logger.debug() << "Parse command:" << command.left(20);

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

  if (m_state == eInitializing && type == "status") {
    m_state = eReady;

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

  if (m_state != eReady) {
    logger.error() << "Unexpected command";
    return;
  }

  if (type == "status") {
    QJsonValue serverIpv4Gateway = obj.value("serverIpv4Gateway");
    if (!serverIpv4Gateway.isString()) {
      logger.error() << "Unexpected serverIpv4Gateway value";
      return;
    }

    QJsonValue deviceIpv4Address = obj.value("deviceIpv4Address");
    if (!deviceIpv4Address.isString()) {
      logger.error() << "Unexpected deviceIpv4Address value";
      return;
    }

    QJsonValue txBytes = obj.value("txBytes");
    if (!txBytes.isDouble()) {
      logger.error() << "Unexpected txBytes value";
      return;
    }

    QJsonValue rxBytes = obj.value("rxBytes");
    if (!rxBytes.isDouble()) {
      logger.error() << "Unexpected rxBytes value";
      return;
    }

    emit statusUpdated(serverIpv4Gateway.toString(),
                       deviceIpv4Address.toString(), txBytes.toDouble(),
                       rxBytes.toDouble());
    return;
  }

  if (type == "disconnected") {
    emit disconnected();
    return;
  }

  if (type == "connected") {
    QString pubkey = obj.value("pubkey").toString();
    logger.debug() << "Handshake completed with:" << pubkey;

    if (m_activationQueue.isEmpty()) {
      return;
    }
    const HopConnection& hop = m_activationQueue.first();
    if (hop.m_server.publicKey() != pubkey) {
      return;
    }

    // After a connection is completed, start the next handshake or signal
    // success if all connections came up successfully.
    m_activationQueue.removeFirst();
    if (m_activationQueue.isEmpty()) {
      emit connected();
    } else {
      activateNext();
    }
    return;
  }

  if (type == "backendFailure") {
    MozillaVPN::instance()->errorHandle(ErrorHandler::ControllerError);
    return;
  }

  if (type == "logs") {
    // We don't care if we are not waiting for logs.
    if (!m_logCallback) {
      return;
    }

    QJsonValue logs = obj.value("logs");
    m_logCallback(logs.isString() ? logs.toString().replace("|", "\n")
                                  : QString());
    m_logCallback = nullptr;
    return;
  }

  logger.warning() << "Invalid command received:" << command;
}

void LocalSocketController::write(const QJsonObject& json) {
  Q_ASSERT(m_socket);
  m_socket->write(QJsonDocument(json).toJson(QJsonDocument::Compact));
  m_socket->write("\n");
}
