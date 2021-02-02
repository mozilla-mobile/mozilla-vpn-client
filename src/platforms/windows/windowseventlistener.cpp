/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowseventlistener.h"
#include "logger.h"
#include "mozillavpn.h"
#include "platforms/windows/windowscommons.h"
#include "qmlengineholder.h"

#include <QLocalSocket>

#include <Windows.h>

constexpr const char* UI_PIPE = "\\\\.\\pipe\\mozillavpn.ui";

namespace {
Logger logger(LOG_WINDOWS, "WindowsEventListener");
}

WindowsEventListener::WindowsEventListener() {
  logger.log() << "Windows event listener created";

  m_server.setSocketOptions(QLocalServer::UserAccessOption);

  logger.log() << "Server path:" << UI_PIPE;

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

WindowsEventListener::~WindowsEventListener() {
  logger.log() << "Windows event listener released";
}

bool WindowsEventListener::checkOtherInstances() {
  logger.log() << "Checking other instances";

  // Let's check if there is a window with the right name.
  QString windowTitle = qtTrId("vpn.main.productName");
  HWND window = FindWindow(nullptr, (const wchar_t*)windowTitle.utf16());
  if (!window) {
    WindowsCommons::windowsLog("No other instances found");
    return true;
  }

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
