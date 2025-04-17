/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

#include "webextbridge.h"
#include "webexthandler.h"
#include "webextreader.h"

const QStringList ALLOW_LISTED_WEBEXTENSIONS = {
    "@testpilot-containers",
    "vpn@mozilla.com"
};

constexpr const quint16 VPN_WEBEXTENSION_PORT = 8754;

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("mozillavpn-webext-bridge");
  QCoreApplication::setApplicationVersion("0.1");

  QStringList args = app.arguments();
  if (args.length() != 3) { 
    qWarning() << "Expected 2 arguments got:" << args.length() - 1;
    qInfo() << "Please invoke using <manifest path> <extension id>";
    return 1;
  }
  const QString extId = args.last();
  if (!ALLOW_LISTED_WEBEXTENSIONS.contains(extId)) {
    qWarning() << app.applicationName() << "is not accessible for extension:"
               << extId;
    return 1;
  }

  QFile stdoutFile;
  stdoutFile.open(stdout, QIODeviceBase::WriteOnly);
  WebExtHandler handler(&stdoutFile);

  // Create a webextension bridge to the VPN client.
  WebExtBridge bridge(VPN_WEBEXTENSION_PORT);
  QObject::connect(&bridge, &WebExtBridge::messageReceived, &handler,
                   &WebExtHandler::writeMsgStdout);
  QObject::connect(&handler, &WebExtHandler::unhandledMessage, &bridge,
                   &WebExtBridge::sendMessage);
  QObject::connect(&bridge, &WebExtBridge::connected, &handler, [&]() {
    handler.writeStatus("vpn-client-up");
  });
  QObject::connect(&bridge, &WebExtBridge::disconnected, &handler, [&]() {
    handler.writeStatus("vpn-client-down");
  });

  // Read messages from stdin and deliver them to the handler.
  QFile stdinFile;
  stdinFile.open(stdin, QIODevice::ReadOnly);
  WebExtReader stdinReader(&stdinFile);
  QObject::connect(&stdinReader, &WebExtReader::messageReceived, &handler,
                   &WebExtHandler::handleMessage);
  QObject::connect(&stdinReader, &WebExtReader::eofReceived, &app,
                   &QCoreApplication::quit);

  // Run the web extension bridge.
  QObject::connect(qApp, &QCoreApplication::aboutToQuit,
    []() { qInfo() << "Shutting down"; });
  return app.exec();
}
