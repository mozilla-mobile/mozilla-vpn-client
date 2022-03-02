/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtutorial.h"
#include "../../src/tutorial.h"
#include "../../src/qmlengineholder.h"

void TestTutorial::parser_data() {
  QTest::addColumn<QByteArray>("input");
  QTest::addColumn<bool>("result");

  QTest::addRow("empty") << QByteArray("") << false;
  QTest::addRow("empty lines") << QByteArray("\n\n\n") << false;
  QTest::addRow("empty lines with spaces")
      << QByteArray(" \n   \n\t \n ") << false;
  QTest::addRow("comments")
      << QByteArray("# this is a comment\n # Here another one") << false;
  QTest::addRow("invalid command") << QByteArray("T") << false;
  QTest::addRow("tooltip invalid") << QByteArray("TOOLTIP") << false;
  QTest::addRow("tooltip invalid2") << QByteArray("TOOLTIP ") << false;
  QTest::addRow("tooltip invalid3") << QByteArray("TOOLTIP a") << false;
  QTest::addRow("tooltip invalid3") << QByteArray("TOOLTIP a ") << false;
  QTest::addRow("tooltip invalid stringID")
      << QByteArray("TOOLTIP a b") << false;
  QTest::addRow("tooltip valid")
      << QByteArray("TOOLTIP ServersViewSearchPlaceholder aaa") << true;
}

void TestTutorial::parser() {
  QmlEngineHolder qml;

  Tutorial* t = Tutorial::instance();
  QVERIFY(!!t);
  QVERIFY(!t->isPlaying());

  QSignalSpy signalSpy(t, &Tutorial::playingChanged);

  t->stop();
  QCOMPARE(signalSpy.count(), 0);

  QFETCH(QByteArray, input);
  QTemporaryFile file;
  QVERIFY(file.open());
  QCOMPARE(file.write(input.data(), input.length()), input.length());
  file.close();

  QFETCH(bool, result);

  t->play(file.fileName());
  QCOMPARE(signalSpy.count(), result ? 1 : 0);

  if (result) {
    t->stop();
    QCOMPARE(signalSpy.count(), 2);
  }
}

static TestTutorial s_testTutorial;
