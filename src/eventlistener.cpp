/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "eventlistener.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>
#include <QUrl>
#ifndef MZ_WINDOWS
#  include <QStandardPaths>
#  include <QRegularExpression>
#endif

#include "constants.h"
#include "frontend/navigator.h"
#include "logger.h"
#include "qmlengineholder.h"

#if defined(MZ_WINDOWS)
#  include <windows.h>

#  include "platforms/windows/windowsutils.h"
#endif

namespace {
Logger logger("EventListener");
}

EventListener::EventListener() {
  logger.debug() << " event listener created";

  m_server.setSocketOptions(QLocalServer::UserAccessOption);

#if defined(MZ_WINDOWS)
  m_pipeLocation = QString("\\\\.\\pipe\\%1.ui").arg(pipeName());
#else
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
  m_pipeLocation = dir.filePath(QString("%1.ui.sock").arg(pipeName()));
#endif

  logger.debug() << "Server path:" << m_pipeLocation;

#ifdef MZ_LINUX
  if (QFileInfo::exists(m_pipeLocation)) {
    QFile::remove(m_pipeLocation);
  }
#endif

  if (!m_server.listen(m_pipeLocation)) {
    logger.error() << "Failed to listen the daemon path";
    return;
  }

  connect(&m_server, &QLocalServer::newConnection, &m_server, [&] {
    logger.debug() << "New connection received";

    if (!m_server.hasPendingConnections()) {
      return;
    }

    QLocalSocket* socket = m_server.nextPendingConnection();
    Q_ASSERT(socket);

    connect(socket, &QLocalSocket::readyRead, this,
            &EventListener::socketReadyRead);
    connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
  });
}

void EventListener::socketReadyRead() {
  QObject* obj = QObject::sender();
  QLocalSocket* socket = qobject_cast<QLocalSocket*>(obj);
  if (socket == nullptr) {
    logger.warning() << "Signal sender is not a socket!";
    return;
  }

  QString input = QString::fromUtf8(socket->readLine()).trimmed();
  logger.debug() << "EventListener input:" << input;
  QString command = input.section(' ', 0, 0);
  QString payload = input.section(' ', 1, -1);

  if (command == "show") {
    QmlEngineHolder* engine = QmlEngineHolder::instance();
    engine->showWindow();
  } else if (command == "link") {
    handleLinkCommand(payload);
  } else {
    logger.info() << "Unknown UI command:" << command;
  }
}

void EventListener::handleLinkCommand(const QString& payload) {
  const QUrl url(payload);

  // We only accept the mozilla-vpn scheme.
  if (url.scheme() != Constants::DEEP_LINK_SCHEME) {
    return;
  }

  // The URL authority determines who handles this.
  if (url.authority() == "nav") {
    Navigator::instance()->requestDeepLink(url);
  } else {
    logger.info() << "Unknown deep link target:" << url.authority();
  }

  // Show the window after handling a deep link.
  QmlEngineHolder* engine = QmlEngineHolder::instance();
  engine->showWindow();
}

EventListener::~EventListener() {
  logger.debug() << " event listener released";

  m_server.close();

#ifdef MZ_LINUX
  if (QFileInfo::exists(m_pipeLocation)) {
    QFile::remove(m_pipeLocation);
  }
#endif
}

bool EventListener::checkForInstances(const QString& windowName) {
  logger.debug() << "Checking other instances";

#ifdef MZ_WINDOWS
  // Let's check if there is a window with the right name.
  HWND window =
      FindWindow(nullptr, reinterpret_cast<const wchar_t*>(windowName.utf16()));
  if (!window) {
    WindowsUtils::windowsLog("No other instances found");
    return false;
  }
#endif

  // Try to wake the UI and bring it to the foreground.
  logger.debug() << "Try to communicate with the existing instance";
  return sendCommand("show");
}

bool EventListener::sendCommand(const QString& message) {
#if defined(MZ_WINDOWS)
  QString path = QString("\\\\.\\pipe\\%1.ui").arg(pipeName());
#else
  QString path = QStandardPaths::locate(QStandardPaths::RuntimeLocation,
                                        QString("%1.ui.sock").arg(pipeName()));
  if (path.isEmpty()) {
    return false;
  }
#endif

  QLocalSocket socket;
  socket.connectToServer(path);
  if (!socket.waitForConnected(1000)) {
    logger.error() << "Connection failed:" << socket.errorString();
    return false;
  }

  QByteArray data = message.toUtf8();
  if (!message.endsWith('\n')) {
    data.append('\n');
  }
  socket.write(data);
  socket.disconnectFromServer();
  if (socket.state() != QLocalSocket::UnconnectedState) {
    socket.waitForDisconnected(1000);
  }
  return true;
}

bool EventListener::sendDeepLink(const QUrl& url) {
  QString message = QString("link %1").arg(url.toString(QUrl::FullyEncoded));
  return sendCommand(message);
}

// static
QString EventListener::pipeName() {
  QString appName = qApp->applicationName().toLower();
  return appName.remove(QRegularExpression("[^a-z]"));
}
