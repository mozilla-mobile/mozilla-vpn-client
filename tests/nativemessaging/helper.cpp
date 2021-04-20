/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

QVector<QObject*> TestHelper::s_testList;
QProcess* TestHelper::s_nativeMessagingProcess = nullptr;

TestHelper::TestHelper() { s_testList.append(this); }

// static
void TestHelper::runNativeMessaging(const char* app) {
  Q_ASSERT(s_nativeMessagingProcess == nullptr);

  s_nativeMessagingProcess = new QProcess();
  s_nativeMessagingProcess->setReadChannel(QProcess::StandardOutput);

  s_nativeMessagingProcess->start(app, QStringList(),
                                  QProcess::Unbuffered | QProcess::ReadWrite);
  if (!s_nativeMessagingProcess->waitForStarted()) {
    qFatal("Failed to start the naive messaging process");
  }
}

// static
void TestHelper::killNativeMessaging() {
  Q_ASSERT(s_nativeMessagingProcess);

  s_nativeMessagingProcess->closeWriteChannel();
  s_nativeMessagingProcess->closeReadChannel(QProcess::StandardOutput);
  s_nativeMessagingProcess->closeReadChannel(QProcess::StandardError);

  s_nativeMessagingProcess->terminate();

  if (!s_nativeMessagingProcess->waitForFinished()) {
    qFatal("Failed to kill the native messaging process");
  }

  s_nativeMessagingProcess->deleteLater();
  s_nativeMessagingProcess = nullptr;
}

int TestHelper::runTests(const char* app) {
  int failures = 0;

  for (QObject* obj : TestHelper::s_testList) {
    runNativeMessaging(app);

    int result = QTest::qExec(obj);
    if (result != 0) {
      ++failures;
    }

    killNativeMessaging();
  }

  return failures;
}

bool TestHelper::write(const QByteArray& data) {
  Q_ASSERT(s_nativeMessagingProcess);

  uint32_t length = (uint32_t)data.length();
  char* rawLength = reinterpret_cast<char*>(&length);

  return s_nativeMessagingProcess->write(rawLength, sizeof(uint32_t)) ==
             sizeof(uint32_t) &&
         s_nativeMessagingProcess->write(data.constData(), length) == length &&
         s_nativeMessagingProcess->waitForBytesWritten();
}

QByteArray TestHelper::read() {
  Q_ASSERT(s_nativeMessagingProcess);

  s_nativeMessagingProcess->waitForReadyRead();

  char rawLength[sizeof(uint32_t)];
  if (s_nativeMessagingProcess->read(rawLength, sizeof(uint32_t)) !=
      sizeof(uint32_t)) {
    return QByteArray();
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  return s_nativeMessagingProcess->read(length);
}

QByteArray TestHelper::tryToRead() {
  Q_ASSERT(s_nativeMessagingProcess);

  s_nativeMessagingProcess->waitForReadyRead(200);

  char rawLength[sizeof(uint32_t)];
  if (s_nativeMessagingProcess->read(rawLength, sizeof(uint32_t)) !=
      sizeof(uint32_t)) {
    return QByteArray();
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  return s_nativeMessagingProcess->read(length);
}
