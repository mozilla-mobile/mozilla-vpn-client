/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottie.h"

#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  Lottie::initialize(&engine, "LottieTest 0.1");

  if (argc != 2) {
    qDebug() << "Usage:";
    qDebug() << argv[0] << "<lottie.json>";
    return 1;
  }

  if (!QFile::exists(argv[1])) {
    qDebug() << argv[1] << "does not exist";
    return 1;
  }

  QQmlContext* ctx = engine.rootContext();
  ctx->setContextProperty("INPUTFILE", argv[1]);
  engine.load(QUrl(QStringLiteral("qrc:/lottietest.qml")));

  return app.exec();
}
