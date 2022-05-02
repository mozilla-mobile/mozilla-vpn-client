/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "commandgpucheck.h"
#include "commandlineparser.h"
#include "leakdetector.h"
#include "logger.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QTextStream>

namespace {
Logger logger(LOG_MAIN, "CommandGpuCheck");

uint32_t s_vertexShaderErrorCount = 0;
void messageQTHandler(QtMsgType, const QMessageLogContext&,
                      const QString& message) {
  if (message.contains("0x80070057")) {
    ++s_vertexShaderErrorCount;
  }
}

}  // namespace

CommandGpuCheck::CommandGpuCheck(QObject* parent)
    : Command(parent, "gpucheck", "GPU configuration check") {
  MVPN_COUNT_CTOR(CommandGpuCheck);
}

CommandGpuCheck::~CommandGpuCheck() { MVPN_COUNT_DTOR(CommandGpuCheck); }

int CommandGpuCheck::run(QStringList& tokens) {
  Q_ASSERT(!tokens.isEmpty());

  // Let's take the location before creating QGuiApplication.
  QString location =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

  QGuiApplication app(CommandLineParser::argc(), CommandLineParser::argv());

  qInstallMessageHandler(messageQTHandler);

  QObject::connect(&app, &QCoreApplication::aboutToQuit, [location]() {
    QDir appDataLocation(location);
    if (!appDataLocation.exists()) {
      QDir tmp(location);
      tmp.cdUp();
      if (!tmp.exists()) {
        return;
      }
      if (!tmp.mkdir(appDataLocation.dirName())) {
        return;
      }
    }

    QFile gpuCheckSettings = appDataLocation.filePath("moz.vpn.gpucheck");
    if (s_vertexShaderErrorCount == 0) {
      if (gpuCheckSettings.exists()) {
        gpuCheckSettings.remove();
      }
      return;
    }

    if (!gpuCheckSettings.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return;
    }

    gpuCheckSettings.write("software");
  });

  QQmlApplicationEngine engine;
  const QUrl url("qrc:/gpucheck/main.qml");
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
          QCoreApplication::exit(-1);
        }
      },
      Qt::QueuedConnection);
  engine.load(url);

  return app.exec();
}

static Command::RegistrationProxy<CommandGpuCheck> s_commandGpuCheck;
