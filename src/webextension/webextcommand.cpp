/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webextcommand.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "commandlineparser.h"
#include "leakdetector.h"
#include "webextbridge.h"
#include "webexthandler.h"

const QStringList ALLOW_LISTED_WEBEXTENSIONS = {
    "@testpilot-containers",
    "vpn@mozilla.com"
};

WebExtCommand::WebExtCommand(QObject* parent)
    : Command(parent, "webext", "Run in web extension/native messaging mode") {
  MZ_COUNT_CTOR(WebExtCommand);
}

WebExtCommand::~WebExtCommand() { MZ_COUNT_DTOR(WebExtCommand); }

int WebExtCommand::run(QStringList& tokens) {
  int argc = tokens.count();
  char** argv = (char**)calloc(sizeof(char*), argc);
  for (int i = 0; i < argc; i++) {
#ifdef Q_OS_WIN
    argv[i] = _strdup(qPrintable(tokens.at(i)));
#else
    argv[i] = strdup(qPrintable(tokens.at(i)));
#endif
  }
  QCoreApplication app(argc, argv);

  // Prepare to parse the command line arguments.
  QCommandLineParser parser;
  const QString desc = "Web extension bridge to the Mozilla VPN client";
  parser.setApplicationDescription(desc);
  parser.addPositionalArgument("manifest", "Path to the extension manifest");
  parser.addPositionalArgument("ext-id", "The extension idedentifier");

  QCommandLineOption optVersion = parser.addVersionOption();
  QCommandLineOption optHelp = parser.addHelpOption();
  QCommandLineOption optName(QStringList({"n", "name"}),
                             "Local socket of the Mozilla VPN client", "NAME");
  optName.setDefaultValue("mozillavpn.webext");
  parser.addOption(optName);

  // Parse command line arguments.
  parser.parse(tokens);
  if (parser.isSet(optHelp)) {
    parser.showHelp();
    return 0;
  }
  if (parser.isSet(optVersion)) {
    parser.showVersion();
    return 0;
  }

  QStringList args = parser.positionalArguments();
  if (args.length() != 2) { 
    qWarning() << "Expected 2 arguments got:" << args.length();
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
  QObject::connect(&handler, &WebExtHandler::eofReceived, &app,
                   &QCoreApplication::quit);

  // Create a webextension bridge to the VPN client.
  WebExtBridge bridge(parser.value(optName));
  QObject::connect(&bridge, &WebExtBridge::messageReceived, &handler,
                   &WebExtHandler::writeMsgStdout);
  QObject::connect(&handler, &WebExtHandler::unhandledMessage, &bridge,
                   [&](const QByteArray msg) {
                     if (!bridge.sendMessage(msg)) {
                       QJsonObject obj({{"error", "vpn-client-down"}});
                       handler.writeJsonStdout(obj);
                     }
                    });
  QObject::connect(&bridge, &WebExtBridge::connected, &handler, [&]() {
    qInfo() << "Connected!";
    handler.writeJsonStdout(QJsonObject({{"status", "vpn-client-up"}}));
  });
  QObject::connect(&bridge, &WebExtBridge::disconnected, &handler, [&]() {
    handler.writeJsonStdout(QJsonObject({{"error", "vpn-client-down"}}));
  });

  // Run the web extension bridge.
  QObject::connect(qApp, &QCoreApplication::aboutToQuit,
    []() { qInfo() << "Shutting down"; });
  return app.exec();
}

