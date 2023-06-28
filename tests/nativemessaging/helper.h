/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPER_H
#define HELPER_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QVector>
#include <QtTest/QtTest>

class TestHelper : public QObject {
  Q_OBJECT

 public:
  TestHelper();

  bool waitForConnection();

  bool write(const QByteArray& data);

  QByteArray read();
  QByteArray readIgnoringStatus();

  static int runTests(const char* app);

 protected:
  static void runNativeMessaging(const char* app, QStringList arguments);
  static void killNativeMessaging();

 public:
  static const char* s_app;
  static QVector<QObject*> s_testList;

  static QProcess* s_nativeMessagingProcess;
};

#endif  // HELPER_H
