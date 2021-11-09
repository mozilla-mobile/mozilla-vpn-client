/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>

// For info on the slots we can use
// https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
class Setup : public QObject {
  Q_OBJECT

 public:
  Setup() {}

 public slots:
  void qmlEngineAvailable(QQmlEngine* engine) {
    engine->rootContext()->setContextProperty("myContextProperty",
                                              QVariant(true));
  }
};

// The name qml_tests is arbitrary
QUICK_TEST_MAIN_WITH_SETUP(qml_tests, Setup)

// I don't understand why we need this line, but the docs say we do:
// https://doc.qt.io/qt-5/qtquicktest-index.html#executing-c-before-qml-tests
#include "tst_main.moc"