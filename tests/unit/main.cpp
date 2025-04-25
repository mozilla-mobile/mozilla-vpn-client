/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QCoreApplication>

#include "constants.h"
#include "glean/mzglean.h"
#include "helper.h"
#include "i18nstrings.h"
#include "leakdetector.h"
#include "loghandler.h"
#include "networkrequest.h"
#include "settingsholder.h"

#ifdef _WIN32
#  include <windows.h>

#  include <chrono>
#  include <thread>
#else
#  include <unistd.h>

#  include <cstdio>
#endif

QVector<TestHelper::NetworkConfig> TestHelper::networkConfig;
Controller::State TestHelper::controllerState = Controller::StateInitializing;
QVector<QObject*> TestHelper::testList;
TestHelper::SystemNotification TestHelper::lastSystemNotification;

QObject* TestHelper::findTest(const QString& name) {
  for (QObject* obj : TestHelper::testList) {
    const QMetaObject* meta = obj->metaObject();
    if (meta->className() == name) {
      return obj;
    }
  }

  return nullptr;
}

void maybeWaitForDebugger(int argc, char* argv[]) {
  bool debuggerFlag = false;
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--debugger") {
      debuggerFlag = true;
      break;
    }
  }
  if (!debuggerFlag) {
    // Not asked to wait for a debugger; just return.
    return;
  }
#ifdef _WIN32
  DWORD pid = GetCurrentProcessId();
#else
  pid_t pid = getpid();
#endif
  qDebug() << "PID: " << pid << " - waiting for debugger to attach...\n";

#ifdef _WIN32
  // Use Win32 API to detect debugger
  while (!IsDebuggerPresent()) {
    // Sleep 1 second before checking again
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  qDebug() << "Debugger attached. Continuing...\n";

  return;
#else
  // TODO: Check if that also works on macos
  while (true) {
    QFile file("/proc/self/status");
    auto closeFile = qScopeGuard([&]() { file.close(); });

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;
    }
    while (!file.atEnd()) {
      QString line = file.readLine();
      if (line.startsWith("TracerPid:")) {
        QString pidStr = line.mid(QString("TracerPid:").length()).trimmed();
        if (pidStr.toInt() != 0) {
          qDebug() << "Debugger attached. Continuing...\n";
          return;
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
#endif
}

TestHelper::TestHelper() { testList.append(this); }

// static
App* App::instance() {
  static App* app = nullptr;

  if (!app) {
    app = new App(qApp);
  }

  return app;
}

// static
bool TestHelper::networkRequestGeneric(NetworkRequest* request) {
  Q_ASSERT(!TestHelper::networkConfig.isEmpty());
  TestHelper::NetworkConfig nc = TestHelper::networkConfig.takeFirst();

  QTimer::singleShot(0, request, [request, nc]() {
    request->deleteLater();

    if (nc.m_status == TestHelper::NetworkConfig::Failure) {
      emit request->requestFailed(
          QNetworkReply::NetworkError::HostNotFoundError, "");
    } else {
      Q_ASSERT(nc.m_status == TestHelper::NetworkConfig::Success);

      emit request->requestCompleted(nc.m_body);
    }
  });

  return true;
}

int main(int argc, char* argv[]) {
  maybeWaitForDebugger(argc, argv);
#ifdef MZ_DEBUG
  LeakDetector leakDetector;
  Q_UNUSED(leakDetector);
#endif
  QCoreApplication::setApplicationName("Mozilla VPN Unit Tests");
  QCoreApplication::setOrganizationName("Mozilla Testing");
  QCoreApplication::setApplicationVersion(Constants::versionString());

  {
    SettingsHolder settingsHolder;
    Constants::setStaging();
  }

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  pe.insert("LANG", "en");
  pe.insert("LANGUAGE", "en");

  QCoreApplication app(argc, argv);

  int failures = 0;

  NetworkRequest::setRequestHandler(
      TestHelper::networkRequestDelete, TestHelper::networkRequestGet,
      TestHelper::networkRequestPost, TestHelper::networkRequestPostIODevice);

  I18nStrings::initialize();
  LogHandler::setStderr(true);
  MZGlean::registerLogHandler(LogHandler::rustMessageHandler);

  // If arguments were passed, then run a subset of tests.
  QStringList args = app.arguments();
  if (args.count() > 1) {
    args.removeFirst();
    for (const QString& x : args) {
      QObject* obj = TestHelper::findTest(x);
      if (obj == nullptr) {
        qWarning() << "No such test found:" << x;
        ++failures;
        continue;
      }
      int result = QTest::qExec(obj);
      if (result != 0) {
        ++failures;
      }
    }
  } else {
    // Otherwise, run all the tests.
    for (QObject* obj : TestHelper::testList) {
      int result = QTest::qExec(obj);
      if (result != 0) {
        ++failures;
      }
    }
  }

  return failures;
}
