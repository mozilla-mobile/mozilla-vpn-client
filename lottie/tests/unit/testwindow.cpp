/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testwindow.h"
#include "../../lib/lottieprivate.h"
#include "../../lib/lottieprivatewindow.h"

#include <QJSEngine>
#include <QTimer>

void TestWindow::setInterval() {
  QJSEngine engine;
  LottiePrivate::initialize(&engine, "Foo 1.0");

  LottiePrivate p;
  LottiePrivateWindow window(&p);

  QJSValue globalObject = engine.globalObject();
  globalObject.setProperty("window", engine.toScriptValue(&window));

  // Not callable.
  {
    QJSValue callback;
    int id = window.setInterval(callback, 0);
    QCOMPARE(id, 1);
  }

  // An interval callback
  {
    QEventLoop loop;
    connect(&window, &LottiePrivateWindow::lottieChanged,
            [&]() { loop.exit(); });

    QJSValue callback = engine.evaluate("(function() { window.lottie = 1; })");
    QVERIFY(callback.isCallable());

    int id = window.setInterval(callback, 0);
    QCOMPARE(id, 2);

    loop.exec();

    QVERIFY(window.lottie().isNumber());
    QCOMPARE(window.lottie().toInt(), 1);
  }

  // An interval callback + cancel
  {
    QEventLoop loop;
    connect(&window, &LottiePrivateWindow::lottieChanged,
            [&]() { loop.exit(); });

    QJSValue callback = engine.evaluate("(function() { window.lottie = 2; })");
    QVERIFY(callback.isCallable());

    int id = window.setInterval(callback, 0);
    QCOMPARE(id, 3);
    window.clearInterval(id);

    QTimer timer;
    connect(&timer, &QTimer::timeout, [&]() { loop.exit(); });
    timer.start(100);

    loop.exec();

    QVERIFY(window.lottie().isNumber());
    QCOMPARE(window.lottie().toInt(), 1);
  }
}

void TestWindow::setTimeout() {
  QJSEngine engine;
  LottiePrivate::initialize(&engine, "Foo 1.0");

  LottiePrivate p;
  LottiePrivateWindow window(&p);

  QJSValue globalObject = engine.globalObject();
  globalObject.setProperty("window", engine.toScriptValue(&window));

  // Not callable.
  {
    QJSValue callback;
    int id = window.setTimeout(callback, 0);
    QCOMPARE(id, 1);
  }

  engine.evaluate("window.lottie = 1;");
  QVERIFY(window.lottie().isNumber());
  QCOMPARE(window.lottie().toInt(), 1);

  // An interval callback
  {
    QEventLoop loop;
    connect(&window, &LottiePrivateWindow::lottieChanged, [&]() {
      QVERIFY(window.lottie().isNumber());
      if (window.lottie().toInt() == 4) {
        loop.exit();
      }
    });

    QJSValue callback = engine.evaluate("(function() { ++window.lottie; })");
    QVERIFY(callback.isCallable());

    int id = window.setTimeout(callback, 0);
    QCOMPARE(id, 2);

    loop.exec();

    window.clearTimeout(id);

    QVERIFY(window.lottie().isNumber());
    QCOMPARE(window.lottie().toInt(), 4);
  }

  // An interval callback + cancel
  {
    QEventLoop loop;
    connect(&window, &LottiePrivateWindow::lottieChanged,
            [&]() { loop.exit(); });

    QJSValue callback = engine.evaluate("(function() { ++window.lottie; })");
    QVERIFY(callback.isCallable());

    int id = window.setTimeout(callback, 0);
    QCOMPARE(id, 3);
    window.clearTimeout(id);

    QTimer timer;
    connect(&timer, &QTimer::timeout, [&]() { loop.exit(); });
    timer.start(100);

    loop.exec();

    QVERIFY(window.lottie().isNumber());
    QCOMPARE(window.lottie().toInt(), 4);
  }
}

void TestWindow::suspendAndResume() {
  QJSEngine engine;
  LottiePrivate::initialize(&engine, "Foo 1.0");

  LottiePrivate p;
  LottiePrivateWindow window(&p);

  QJSValue globalObject = engine.globalObject();
  globalObject.setProperty("window", engine.toScriptValue(&window));

  engine.evaluate("window.lottie = 1;");
  QVERIFY(window.lottie().isNumber());
  QCOMPARE(window.lottie().toInt(), 1);

  QEventLoop loop;
  QJSValue callback = engine.evaluate("(function() { ++window.lottie; })");
  QVERIFY(callback.isCallable());

  int id = window.setInterval(callback, 0);
  QCOMPARE(id, 1);

  window.suspend();

  QTimer wait;
  connect(&wait, &QTimer::timeout, [&]() {
    QVERIFY(window.lottie().isNumber());
    QCOMPARE(window.lottie().toInt(), 1);
    loop.exit();
  });
  wait.start(500);
  loop.exec();

  window.resume();

  connect(&window, &LottiePrivateWindow::lottieChanged, [&]() {
    QVERIFY(window.lottie().isNumber());
    if (window.lottie().toInt() == 2) {
      loop.exit();
    }
  });
  loop.exec();

  QVERIFY(window.lottie().isNumber());
  QCOMPARE(window.lottie().toInt(), 2);
}

static TestWindow s_testWindow;
