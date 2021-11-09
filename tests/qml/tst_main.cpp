/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>

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

QUICK_TEST_MAIN_WITH_SETUP(mytest, Setup)

#include "tst_main.moc"