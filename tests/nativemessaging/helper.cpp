/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

QVector<QObject*> TestHelper::s_testList;
QProcess* TestHelper::s_nativeMessagingProcess = nullptr;

TestHelper::TestHelper() { s_testList.append(this); }

// static
void TestHelper::runNativeMessaging(const char* app, QStringList arguments = QStringList()) {
  Q_ASSERT(s_nativeMessagingProcess == nullptr);

  s_nativeMessagingProcess = new QProcess();
  s_nativeMessagingProcess->setReadChannel(QProcess::StandardOutput);

  connect(s_nativeMessagingProcess, &QProcess::readyReadStandardError, []() {
    qDebug() << "[mozillavpnnp - stderr]"
             << s_nativeMessagingProcess->readAllStandardError();
  });
  s_nativeMessagingProcess->start(app, arguments,
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

#ifdef Q_OS_WIN
  // See https://doc.qt.io/qt-6/qprocess.html#terminate
  s_nativeMessagingProcess->kill();
#else
  s_nativeMessagingProcess->terminate();
#endif

  if (!s_nativeMessagingProcess->waitForFinished()) {
    qFatal("Failed to kill the native messaging process");
  }

  s_nativeMessagingProcess->deleteLater();
  s_nativeMessagingProcess = nullptr;
}

int TestHelper::runTests(const char* app) {
  int failures = 0;
  s_app=app;
  for (QObject* obj : TestHelper::s_testList) {
    QStringList args; 
    args.append("/some/url/to/manifest.json");
    // A valid extension id.
    args.append("@testpilot-containers");
    
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

  while (!s_nativeMessagingProcess->bytesAvailable()) {
    s_nativeMessagingProcess->waitForReadyRead();
  }

  char rawLength[sizeof(uint32_t)];
  if (s_nativeMessagingProcess->read(rawLength, sizeof(uint32_t)) !=
      sizeof(uint32_t)) {
    return QByteArray();
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  return s_nativeMessagingProcess->read(length);
}

QByteArray TestHelper::readIgnoringStatus() {
  while (true) {
    QByteArray r = read();
    if (r != "{\"status\":\"vpn-client-down\"}") {
      return r;
    }
  }
}

bool TestHelper::waitForConnection() {
  bool connected = false;
  for (int i = 0; i < 10; ++i) {
    if (read() == "{\"status\":\"vpn-client-up\"}") {
      connected = true;
      break;
    }

    QTimer timer;
    timer.setSingleShot(true);
    timer.start(500);

    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &timer, [&] { loop.exit(); });
    loop.exec();
  }
  return connected;
}
