/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsettings.h"

#include "helper.h"
#include "settingsholder.h"

void TestSettings::transactionErrors() {
  SettingsHolder settingsHolder;

  QVERIFY(!settingsHolder.commitTransaction());
  QVERIFY(!settingsHolder.rollbackTransaction());

  QVERIFY(settingsHolder.beginTransaction());
  QVERIFY(!settingsHolder.beginTransaction());
  QVERIFY(settingsHolder.commitTransaction());
  QVERIFY(!settingsHolder.commitTransaction());

  QVERIFY(settingsHolder.beginTransaction());
  QVERIFY(!settingsHolder.beginTransaction());
  QVERIFY(settingsHolder.commitTransaction());
  QVERIFY(!settingsHolder.rollbackTransaction());

  QVERIFY(settingsHolder.beginTransaction());
  QVERIFY(!settingsHolder.beginTransaction());
  QVERIFY(settingsHolder.rollbackTransaction());
  QVERIFY(!settingsHolder.rollbackTransaction());

  QVERIFY(settingsHolder.beginTransaction());
  QVERIFY(!settingsHolder.beginTransaction());
  QVERIFY(settingsHolder.rollbackTransaction());
  QVERIFY(!settingsHolder.commitTransaction());
}

void TestSettings::transactionCommit() {
  SettingsHolder settingsHolder;

  QVERIFY(settingsHolder.beginTransaction());
  settingsHolder.setFoobar("AAA");
  QCOMPARE(settingsHolder.foobar(), "AAA");
  QVERIFY(settingsHolder.commitTransaction());
  QCOMPARE(settingsHolder.foobar(), "AAA");
}

void TestSettings::transactionRollback() {
  SettingsHolder settingsHolder;

  QVERIFY(settingsHolder.beginTransaction());

  settingsHolder.setBarfoo("TOKEN 1");
  QCOMPARE(settingsHolder.barfoo(), "TOKEN 1");

  settingsHolder.setFoobar("AAA");
  settingsHolder.setFoobar("BBB");
  settingsHolder.setFoobar("CCC");
  QCOMPARE(settingsHolder.foobar(), "CCC");

  int count = 0;
  connect(&settingsHolder, &SettingsHolder::foobarChanged, [&]() { ++count; });

  QVERIFY(settingsHolder.rollbackTransaction());

  QCOMPARE(settingsHolder.foobar(), "FOO BAR");
  QCOMPARE(settingsHolder.barfoo(), "TOKEN 1");
  QCOMPARE(count, 1);
}

void TestSettings::transactionRollbackStartup() {
  // This test simulates a rollback at startup from a journal file.

  // Step 1: let's add some entries in the settings
  {
    SettingsHolder settingsHolder;
    settingsHolder.doNotClearOnDTOR();

    QVERIFY(!settingsHolder.recoveredFromJournal());
    settingsHolder.setFoobar("AAA");
    QCOMPARE(settingsHolder.foobar(), "AAA");
    settingsHolder.setBarfoo("TOKEN 1");
    QCOMPARE(settingsHolder.barfoo(), "TOKEN 1");
  }

  // Step 2: let's start a transaction without finalizing it
  {
    SettingsHolder settingsHolder;
    settingsHolder.doNotClearOnDTOR();

    QVERIFY(!settingsHolder.recoveredFromJournal());
    QCOMPARE(settingsHolder.foobar(), "AAA");
    QCOMPARE(settingsHolder.barfoo(), "TOKEN 1");

    QVERIFY(settingsHolder.beginTransaction());
    settingsHolder.setFoobar("BBB");
    QCOMPARE(settingsHolder.foobar(), "BBB");
    settingsHolder.setBarfoo("TOKEN 2");
    QCOMPARE(settingsHolder.barfoo(), "TOKEN 2");
  }

  // Step 3: a new SettingsHolder object will rollback the changes using the
  // journal file.
  {
    SettingsHolder settingsHolder;
    settingsHolder.doNotClearOnDTOR();

    QVERIFY(settingsHolder.recoveredFromJournal());
    QCOMPARE(settingsHolder.foobar(), "AAA");
    QCOMPARE(settingsHolder.barfoo(), "TOKEN 2");

    settingsHolder.setFoobar("BBB");
  }

  // Step 4: we do not use the journal file at the next restart
  {
    SettingsHolder settingsHolder;
    QVERIFY(!settingsHolder.recoveredFromJournal());
    QCOMPARE(settingsHolder.foobar(), "BBB");
    QCOMPARE(settingsHolder.barfoo(), "TOKEN 2");

    settingsHolder.setFoobar("BBB");
  }
}

void TestSettings::sensitiveLogging() {
  SettingsHolder settingsHolder;
  settingsHolder.setFoobar("NotSensitive");
  settingsHolder.setSensitive("Do NOT print this out!");

  QString report;
  // We can do this because SettingsHolder::serializeLogs() is sync.
  settingsHolder.serializeLogs(
      [report = &report](const QString& name, const QString& logs) {
        *report = logs;
      });

  QVERIFY(report.contains("\nfoobar -> NotSensitive\n"));
  QVERIFY(report.contains("\nsensitive -> <Sensitive>\n"));
  QVERIFY(!report.contains("Do NOT print this out!"));
}

static TestSettings s_testSettings;
