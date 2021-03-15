/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidintentreciver.h"
#include "androidcontroller.h"
#include "mozillavpn.h"
#include "logger.h"
#include <QObject>
#include <QApplication>
#include <QtAndroid>
#include <QAndroidIntent>
#include <QTextCodec>
#include "jni.h"

namespace {
AndroidIntentReciver* instance = nullptr;
}

AndroidIntentReciver::AndroidIntentReciver(QObject* parent) : QObject(parent) {
  connect(qApp, &QApplication::applicationStateChanged, this,
          &AndroidIntentReciver::applicationStateChanged);
}

namespace {
Logger logger(LOG_ANDROID, "AndroidIntentReciver");
}

void AndroidIntentReciver::Init() {
  if (instance != nullptr) {
    return;
  }
  instance = new AndroidIntentReciver(qApp);
}

void AndroidIntentReciver::readCurrentIntent() {
  logger.log() << "try to read resume intent";
  auto activity = QtAndroid::androidActivity();
  QAndroidJniObject jIntent =
      activity.callObjectMethod("getIntent", "()Landroid/content/Intent;");
  if (!jIntent.isValid()) {
    logger.log() << "Activity Intent is not readable";
    return;
  }

  auto intent = QAndroidIntent(jIntent);
  auto buffer = intent.extraBytes("command");
  if (buffer.length() == 0) {
    logger.log() << "No Extra data";
    return;
  }
  // Read Buffer as UTF-8
  auto command = QTextCodec::codecForMib(106)->toUnicode(buffer);
  ;

  if (command == "captive-portal-stop") {
    MozillaVPN::instance()->deactivate();
    logger.log() << "Intent requests to stop vpn for captive portal";
    return;
  }
  logger.log() << "Intent Command Unkown: " << command;
}

void AndroidIntentReciver::applicationStateChanged(Qt::ApplicationState state) {
  if (state != Qt::ApplicationState::ApplicationActive) {
    return;
  }
  // In case something made us resume execution, check the intent if there were
  // extra commands
  logger.log() << "Application is resumed";
  readCurrentIntent();
}
