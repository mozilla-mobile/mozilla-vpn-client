/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "eventlistener.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QLocalSocket>

#if defined(MVPN_WINDOWS)
#  include "platforms/windows/windowscommons.h"
#  include <windows.h>

constexpr const char* UI_PIPE = "\\\\.\\pipe\\mozillavpn.ui";
#elif defined(MVPN_LINUX)
#  include <QFileInfo>

constexpr const char* UI_PIPE = "/tmp/mozillavpn.ui.sock";
#endif

namespace {
Logger logger(LOG_MAIN, "EventListener");
}

EventListener::EventListener() {
  logger.log() << " event listener created";

  m_server.setSocketOptions(QLocalServer::UserAccessOption);

  logger.log() << "Server path:" << UI_PIPE;

#ifdef MVPN_LINUX
  if (QFileInfo::exists(UI_PIPE)) {
    QFile::remove(UI_PIPE);
  }
#endif

  if (!m_server.listen(UI_PIPE)) {
    logger.log() << "Failed to listen the daemon path";
    return;
  }

  connect(&m_server, &QLocalServer::newConnection, [&] {
    logger.log() << "New connection received";

    if (!m_server.hasPendingConnections()) {
      return;
    }

    QLocalSocket* socket = m_server.nextPendingConnection();
    Q_ASSERT(socket);

    connect(socket, &QLocalSocket::readyRead, [socket]() {
      QByteArray input = socket->readAll();
      input = input.trimmed();

      logger.log() << "EventListener input:" << input;

      // So far, just the show window signal, but in the future, we could have
      // more.
      if (input == "show") {
        QmlEngineHolder* engine = QmlEngineHolder::instance();
        engine->showWindow();
        return;
      }
    });
  });
}

EventListener::~EventListener() {
  logger.log() << " event listener released";

  m_server.close();

#ifdef MVPN_LINUX
  if (QFileInfo::exists(UI_PIPE)) {
    QFile::remove(UI_PIPE);
  }
#endif
}

bool EventListener::checkOtherInstances() {
  logger.log() << "Checking other instances";

#ifdef MVPN_WINDOWS
  // Let's check if there is a window with the right name.
  QString windowTitle = qtTrId("vpn.main.productName");
  HWND window = FindWindow(nullptr, (const wchar_t*)windowTitle.utf16());
  if (!window) {
    WindowsCommons::windowsLog("No other instances found");
    return true;
  }
#else
  if (!QFileInfo::exists(UI_PIPE)) {
    logger.log() << "No other instances found - no unix socket";
    return true;
  }
#endif

  logger.log() << "Try to communicate with the existing instance";

  QLocalSocket socket;
  socket.connectToServer(UI_PIPE);
  if (!socket.waitForConnected(1000)) {
    logger.log() << "Connection failed.";
    return true;
  }

  logger.log() << "Request to show up";
  socket.write("show\n");

  logger.log() << "Disconnecting";
  socket.disconnectFromServer();
  if (socket.state() != QLocalSocket::UnconnectedState) {
    socket.waitForDisconnected(1000);
  }

  logger.log() << "Terminating the current process";
  return false;
}
