/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "eventlistener.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>
#include <QRegularExpression>
#include <QStandardPaths>

#include "appconstants.h"
#include "logger.h"
#include "qmlengineholder.h"

#if defined(MZ_WINDOWS)
#  include <windows.h>
#elif defined(MZ_LINUX)
#  include <QFileInfo>
#endif

namespace {
Logger logger("EventListener");
}

EventListener::EventListener() {
  logger.debug() << " event listener created";

  m_server.setSocketOptions(QLocalServer::UserAccessOption);

  logger.debug() << "Server path:" << pipeLocation();

#ifdef MZ_LINUX
  if (QFileInfo::exists(pipeLocation())) {
    QFile::remove(pipeLocation());
  }
#endif

  if (!m_server.listen(pipeLocation())) {
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

    connect(socket, &QLocalSocket::readyRead, socket, [socket]() {
      QByteArray input = socket->readAll();
      input = input.trimmed();

      logger.debug() << "EventListener input:" << input;

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
  logger.debug() << " event listener released";

  m_server.close();

#ifdef MZ_LINUX
  if (QFileInfo::exists(pipeLocation())) {
    QFile::remove(pipeLocation());
  }
#endif
}

bool EventListener::checkOtherInstances() {
  logger.debug() << "Checking other instances";

#ifdef MZ_WINDOWS
  // Let's check if there is a window with the right name.
  QString windowTitle = qtTrId("vpn.main.productName");
  HWND window = FindWindow(nullptr, (const wchar_t*)windowTitle.utf16());
  if (!window) {
    WindowsUtils::windowsLog("No other instances found");
    return true;
  }
#else
  if (!QFileInfo::exists(pipeLocation())) {
    logger.warning() << "No other instances found - no unix socket";
    return true;
  }
#endif

  logger.debug() << "Try to communicate with the existing instance";

  QLocalSocket socket;
  socket.connectToServer(pipeLocation());
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

QString EventListener::pipeLocation() {
  QString appName = qApp->applicationName().toLower();
  QString appNameSimple = appName.remove(QRegularExpression("[^a-z]"));

#if defined(MZ_WINDOWS)
  return QString("\\\\.\\pipe\\%1.ui").arg(appNameSimple);
#else
  static QString path;
  if (path.isEmpty()) {
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
    path = dir.filePath(QString("%1.ui.sock").arg(appNameSimple));
  }

  return path;
#endif
}
