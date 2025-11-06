/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

QVector<QObject*> TestHelper::s_testList;
char* TestHelper::s_app = nullptr;

TestHelper::TestHelper() {
  s_testList.append(this);

  m_nativeMessagingProcess.setReadChannel(QProcess::StandardOutput);
  connect(&m_nativeMessagingProcess, &QProcess::readyReadStandardError, [&]() {
    qDebug() << "[mozillavpnnp - stderr]"
             << m_nativeMessagingProcess.readAllStandardError();
  });
}

void TestHelper::init() {
  QStringList args;
  args.append("/some/url/to/mozillavpn.json");
  args.append("@testpilot-containers");

  runNativeMessaging(args);
}

void TestHelper::cleanup() {
  killNativeMessaging();
}

// static
void TestHelper::runNativeMessaging(QStringList arguments) {
  arguments.prepend("webext");
  qDebug() << "run with args:" << arguments;

  m_nativeMessagingProcess.start(s_app, arguments,
                                 QProcess::Unbuffered | QProcess::ReadWrite);
  if (!m_nativeMessagingProcess.waitForStarted()) {
    qFatal("Failed to start the naive messaging process");
  }
}

// static
void TestHelper::killNativeMessaging() {
  if (m_nativeMessagingProcess.state() != QProcess::Running) {
    return;
  }
  m_nativeMessagingProcess.closeWriteChannel();
  m_nativeMessagingProcess.closeReadChannel(QProcess::StandardOutput);
  m_nativeMessagingProcess.closeReadChannel(QProcess::StandardError);

#ifdef Q_OS_WIN
  // See https://doc.qt.io/qt-6/qprocess.html#terminate
  m_nativeMessagingProcess.kill();
#else
  m_nativeMessagingProcess.terminate();
#endif

  if (!m_nativeMessagingProcess.waitForFinished()) {
    qFatal("Failed to kill the native messaging process");
  }
}

int TestHelper::runTests(char* app) {
  int failures = 0;
  s_app = app;
  for (QObject* obj : TestHelper::s_testList) {
    int result = QTest::qExec(obj);
    if (result != 0) {
      ++failures;
    }
  }

  return failures;
}

bool TestHelper::write(const QByteArray& data) {
  uint32_t length = (uint32_t)data.length();
  char* rawLength = reinterpret_cast<char*>(&length);

  return m_nativeMessagingProcess.write(rawLength, sizeof(uint32_t)) ==
             sizeof(uint32_t) &&
         m_nativeMessagingProcess.write(data.constData(), length) == length &&
         m_nativeMessagingProcess.waitForBytesWritten();
}

QByteArray TestHelper::read() {
  while (!m_nativeMessagingProcess.bytesAvailable()) {
    m_nativeMessagingProcess.waitForReadyRead();
  }

  char rawLength[sizeof(uint32_t)];
  if (m_nativeMessagingProcess.read(rawLength, sizeof(uint32_t)) !=
      sizeof(uint32_t)) {
    return QByteArray();
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  return m_nativeMessagingProcess.read(length);
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
