/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "eventlistener.h"

#include <QFileInfo>
#include <QLocalSocket>

#include "constants.h"
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

  logger.debug() << "Server path:" << Constants::UI_PIPE;

#ifdef MZ_LINUX
  if (QFileInfo::exists(Constants::UI_PIPE)) {
    QFile::remove(Constants::UI_PIPE);
  }
#endif

  if (!m_server.listen(Constants::UI_PIPE)) {
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

    connect(socket, &QLocalSocket::readyRead, this, &EventListener::socketReadyRead);
  });
}

void EventListener::socketReadyRead() {
  QObject* obj = QObject::sender();
  QLocalSocket* socket = qobject_cast<QLocalSocket*>(obj);
  QString input = QString::fromUtf8(socket->readLine()).trimmed();

  logger.debug() << "EventListener input:" << input;
  QString command = input.section(' ', 0, 0);
  QString payload = input.section(' ', 1, -1);

  if (command == "show") {
    QmlEngineHolder* engine = QmlEngineHolder::instance();
    engine->showWindow();
  } else {
    logger.debug() << "Unknown UI command:" << command;
  }
}

EventListener::~EventListener() {
  logger.debug() << " event listener released";

  m_server.close();

#ifdef MZ_LINUX
  if (QFileInfo::exists(Constants::UI_PIPE)) {
    QFile::remove(Constants::UI_PIPE);
  }
#endif
}

bool EventListener::checkOtherInstances(const QString& windowName) {
  logger.debug() << "Checking other instances";

#ifdef MZ_WINDOWS
  // Let's check if there is a window with the right name.
  HWND window =
      FindWindow(nullptr, reinterpret_cast<const wchar_t*>(windowName.utf16()));
  if (!window) {
    WindowsUtils::windowsLog("No other instances found");
    return true;
  }
#else
  if (!QFileInfo::exists(Constants::UI_PIPE)) {
    logger.warning() << "No other instances found - no unix socket";
    return true;
  }
#endif

  logger.debug() << "Try to communicate with the existing instance";

  QLocalSocket socket;
  socket.connectToServer(Constants::UI_PIPE);
  if (!socket.waitForConnected(1000)) {
    logger.error() << "Connection failed.";
    return true;
  }

  logger.debug() << "Request to show up";
  socket.write("show\n");

  logger.debug() << "Disconnecting";
  socket.disconnectFromServer();
  if (socket.state() != QLocalSocket::UnconnectedState) {
    socket.waitForDisconnected(1000);
  }

  logger.debug() << "Terminating the current process";
  return false;
}
