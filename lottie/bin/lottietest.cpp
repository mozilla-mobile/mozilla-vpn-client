/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottie.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName("lottietest");
  QGuiApplication::setApplicationVersion("0.1");

  QCommandLineParser parser;
  parser.setApplicationDescription("A simple lottie viewer");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("source", "The lottie input file.");

  QCommandLineOption quitOption(QStringList() << "q"
                                              << "quit",
                                "Quit at the end of the animation");
  parser.addOption(quitOption);

  QCommandLineOption loopOption(
      QStringList() << "l"
                    << "loop",
      "The number of loops. The value must be a number. Use: -1 for infinite.",
      "loop");
  parser.addOption(loopOption);

  QCommandLineOption speedOption(QStringList() << "s"
                                               << "speed",
                                 "Set the speed. The value must be a double.",
                                 "speed");
  parser.addOption(speedOption);

  QCommandLineOption reverseOption(QStringList() << "r"
                                                 << "reverse",
                                   "Reverse");
  parser.addOption(reverseOption);

  QCommandLineOption fillModeOption(
      QStringList() << "f"
                    << "fillMode",
      "Set the fill mode. The possible values: stretch, preserveAspectFit, "
      "preserveAspectCrop, pad. Default: stretch",
      "fillMode");
  parser.addOption(fillModeOption);

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.length() != 1) {
    parser.showHelp();
    return 1;
  }

  if (!QFile::exists(args[0])) {
    qDebug() << args[0] << "does not exist.";
    return 1;
  }

  QQmlApplicationEngine engine;
  Lottie::initialize(&engine, "LottieTest 0.1");

  QQmlContext* ctx = engine.rootContext();
  ctx->setContextProperty("INPUTFILE", args[0]);
  ctx->setContextProperty("QUITATEND", parser.isSet(quitOption));

  if (parser.isSet(loopOption)) {
    QString loopStr = parser.value(loopOption);

    bool ok = false;
    int loop = loopStr.toInt(&ok, 10);
    if (!ok) {
      parser.showHelp();
      return 1;
    }

    if (loop < 0) {
      ctx->setContextProperty("LOOP", true);
    } else {
      ctx->setContextProperty("LOOP", loop);
    }
  } else {
    ctx->setContextProperty("LOOP", false);
  }

  {
    double speed = 1.0;
    if (parser.isSet(speedOption)) {
      QString speedStr = parser.value(speedOption);

      bool ok = false;
      speed = speedStr.toDouble(&ok);
      if (!ok) {
        parser.showHelp();
        return 1;
      }
    }
    ctx->setContextProperty("SPEED", speed);
  }

  ctx->setContextProperty("REVERSE", parser.isSet(reverseOption));

  {
    QString fillMode = "stretch";
    if (parser.isSet(fillModeOption)) {
      fillMode = parser.value(fillModeOption);
      if (fillMode != "stretch" && fillMode != "preserveAspectFit" &&
          fillMode != "preserveAspectCrop" && fillMode != "pad") {
        parser.showHelp();
        return 1;
      }
    }

    ctx->setContextProperty("FILLMODE", fillMode);
  }

  engine.load(QUrl(QStringLiteral("qrc:/lottietest.qml")));

  return app.exec();
}
