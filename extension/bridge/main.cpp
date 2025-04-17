/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCommandLineOption>
#include <QCommandLineParser>
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

  // Prepare to parse the command line arguments.
  QCommandLineParser parser;
  const QString desc = "Web extension bridge to the Mozilla VPN client";
  parser.setApplicationDescription(desc);
  parser.addPositionalArgument("manifest", "Path to the extension manifest");
  parser.addPositionalArgument("ext-id", "The extension idedentifier");

  QCommandLineOption optVersion = parser.addVersionOption();
  QCommandLineOption optHelp = parser.addHelpOption();
  QCommandLineOption optPort(QStringList({"p", "port"}),
                             "Port number of the Mozilla VPN client", "PORT");
  optPort.setDefaultValue(QString::number(VPN_WEBEXTENSION_PORT));
  parser.addOption(optPort);

  // Parse command line arguments.
  parser.parse(app.arguments());
  if (parser.isSet(optHelp)) {
    parser.showHelp();
    return 0;
  }
  if (parser.isSet(optVersion)) {
    parser.showVersion();
    return 0;
  }

  quint16 portNum = VPN_WEBEXTENSION_PORT;
  if (parser.isSet(optPort)) {
    bool okay = false;
    QString vstring = parser.value(optPort);
    ulong value = vstring.toULong(&okay);
    if (!okay || value > UINT16_MAX) {
      qWarning() << "Invalid port specified:" << vstring;
      return 1;
    }
    portNum = value;
  }

  QStringList args = parser.positionalArguments();
  if (args.length() != 2) { 
    qWarning() << "Expected 2 arguments got:" << args.length();
    parser.showHelp();
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
  WebExtBridge bridge(portNum);
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
