/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscontroller.h"
#include "leakdetector.h"
#include "logger.h"
#include "ipaddressrange.h"
#include "models/device.h"
#include "models/keys.h"
#include "models/server.h"
#include "settingsholder.h"

#include <QLocalSocket>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

constexpr const char* WG_INTERFACE = "moz";

namespace {
Logger logger(QStringList{LOG_CONTROLLER, LOG_WINDOWS}, "WindowsController");
}

WindowsController::WindowsController() { MVPN_COUNT_CTOR(WindowsController); }

WindowsController::~WindowsController() { MVPN_COUNT_DTOR(WindowsController); }

void WindowsController::initialize(const Device*device, const Keys* keys)
{
    Q_UNUSED(device);
    Q_UNUSED(keys);


  Q_ASSERT(!m_socket);
  m_socket = new QLocalSocket(this);

  connect(m_socket, &QLocalSocket::connected, [this]() {
		  logger.log() << "Service connected";
    // TODO: status...?
    emit initialized(true, false, QDateTime());
  });

  connect(m_socket, &QLocalSocket::errorOccurred, [this](QLocalSocket::LocalSocketError socketError) {
		  logger.log() << "Service error occurred:" << socketError;
            emit initialized(false, false, QDateTime());
		    });
  connect(m_socket, &QLocalSocket::stateChanged,
          [this](QLocalSocket::LocalSocketState state) {
      logger.log() << "State changed:"<<state;});

  connect(m_socket, &QLocalSocket::readyRead,
          [this]() { logger.log() << "Something to read!";
  logger.log() << m_socket->readAll();
  });

  m_socket->connectToServer("MozillaVPN");
}

void WindowsController::activate(
    const Server& server, const Device* device, const Keys* keys,
    const QList<IPAddressRange>& allowedIPAddressRanges,
    const QList<QString>& vpnDisabledApps, Reason reason) {
  Q_UNUSED(reason);
  Q_UNUSED(vpnDisabledApps);

  logger.log() << "WindowsController activated" << server.hostname();

    if (!m_socket) {
	    emit disconnected();
	    return;
    }

  QByteArray content;
  content.append("[Interface]\nPrivateKey = ");
  content.append(keys->privateKey().toUtf8());
  content.append("\nAddress = ");
  content.append(device->ipv4Address().toUtf8());

  bool ipv6Enabled = SettingsHolder::instance()->ipv6Enabled();
  if (ipv6Enabled) {
    content.append(", ");
    content.append(device->ipv6Address().toUtf8());
  }

  content.append("\nDNS = ");
  content.append(server.ipv4Gateway().toUtf8());

  if (ipv6Enabled) {
    content.append(", ");
    content.append(server.ipv6Gateway().toUtf8());
  }

  content.append("\n\n[Peer]\nPublicKey = ");
  content.append(server.publicKey().toUtf8());
  content.append("\nEndpoint = ");
  content.append(server.ipv4AddrIn().toUtf8());
  content.append(QString(":%1").arg(server.choosePort()).toUtf8());

  /* In theory, we should use the ipv6 endpoint, but wireguard doesn't seem
   * to be happy if there are 2 endpoints.
  if (ipv6Enabled) {
      content.append("\nEndpoint = [");
      content.append(server.ipv6AddrIn().toUtf8());
      content.append(QString("]:%1").arg(serverPort));
  }
  */

  QStringList allowedIPs;
  for (const IPAddressRange& ip : allowedIPAddressRanges) {
    if (ip.type() == IPAddressRange::IPv4 || (ip.type() == IPAddressRange::IPv6 && ipv6Enabled)) {
      allowedIPs.append(ip.toString());
    }
  }
  content.append(
      QString("\nAllowedIPs = %1\n").arg(allowedIPs.join(", ")).toUtf8());

  QTemporaryDir tmpDir;
  if (!tmpDir.isValid()) {
    logger.log() <<"Cannot create a temporary directory";
emit disconnected();
return;
  }

  QDir dir(tmpDir.path());
  QFile file(dir.filePath(QString("%1.conf").arg(WG_INTERFACE)));
  if (!file.open(QIODevice::ReadWrite)) {
    logger.log() <<"Unable to create a file in the temporary folder";
    emit disconnected();
    return;
  }

  qint64 written = file.write(content);
  if (written != content.length()) {
    logger.log() <<"Unable to write the whole configuration file";
    emit disconnected();
    return;
  }

  file.close();

  m_socket->write("ipcconnect=1\n");
  m_socket->write(QString("config=%1\n\n").arg(file.fileName()).toLocal8Bit());
  m_socket->flush();

  emit connected();
}

void WindowsController::deactivate(Reason reason) {
  Q_UNUSED(reason);

  logger.log() << "WindowsController deactivated";

  if (!m_socket) {
    emit disconnected();
  return;
  }
  
  m_socket->write("ipcdisconnect=1\n\n");
  m_socket->flush();
  emit disconnected();
}

void WindowsController::checkStatus() {
  m_socket->write("ipcconnectionstatus=1\n\n");
  m_socket->flush();

  emit statusUpdated("127.0.0.1", 0, 0);
}

void WindowsController::getBackendLogs(
    std::function<void(const QString&)>&& a_callback) {
  std::function<void(const QString&)> callback = std::move(a_callback);
  callback("WindowsController is always happy");
}

void WindowsController::cleanupBackendLogs() {}
