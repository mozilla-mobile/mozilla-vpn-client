/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsettings.h"
#include "../../src/settingsholder.h"
#include "helper.h"

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
  settingsHolder.setTheme("AAA");
  QCOMPARE(settingsHolder.theme(), "AAA");
  QVERIFY(settingsHolder.commitTransaction());
  QCOMPARE(settingsHolder.theme(), "AAA");
}

void TestSettings::transactionRollback() {
  SettingsHolder settingsHolder;

  QVERIFY(settingsHolder.beginTransaction());

  settingsHolder.setToken("TOKEN 1");
  QCOMPARE(settingsHolder.token(), "TOKEN 1");

  settingsHolder.setTheme("AAA");
  settingsHolder.setTheme("BBB");
  settingsHolder.setTheme("CCC");
  QCOMPARE(settingsHolder.theme(), "CCC");

  int count = 0;
  connect(&settingsHolder, &SettingsHolder::themeChanged, [&]() { ++count; });

  QVERIFY(settingsHolder.rollbackTransaction());

  QCOMPARE(settingsHolder.theme(), DEFAULT_THEME);
  QCOMPARE(settingsHolder.token(), "TOKEN 1");
  QCOMPARE(count, 1);
}

void TestSettings::transactionRollbackStartup() {
  // This test simulates a rollback at startup from a journal file.

  // Step 1: let's add some entries in the settings
  {
    SettingsHolder settingsHolder;
    settingsHolder.doNotClearOnDTOR();

    QVERIFY(!settingsHolder.recoveredFromJournal());
    settingsHolder.setTheme("AAA");
    QCOMPARE(settingsHolder.theme(), "AAA");
    settingsHolder.setToken("TOKEN 1");
    QCOMPARE(settingsHolder.token(), "TOKEN 1");
  }

  // Step 2: let's start a transaction without finalizing it
  {
    SettingsHolder settingsHolder;
    settingsHolder.doNotClearOnDTOR();

    QVERIFY(!settingsHolder.recoveredFromJournal());
    QCOMPARE(settingsHolder.theme(), "AAA");
    QCOMPARE(settingsHolder.token(), "TOKEN 1");

    QVERIFY(settingsHolder.beginTransaction());
    settingsHolder.setTheme("BBB");
    QCOMPARE(settingsHolder.theme(), "BBB");
    settingsHolder.setToken("TOKEN 2");
    QCOMPARE(settingsHolder.token(), "TOKEN 2");
  }

  // Step 3: a new SettingsHolder object will rollback the changes using the
  // journal file.
  {
    SettingsHolder settingsHolder;
    settingsHolder.doNotClearOnDTOR();

    QVERIFY(settingsHolder.recoveredFromJournal());
    QCOMPARE(settingsHolder.theme(), "AAA");
    QCOMPARE(settingsHolder.token(), "TOKEN 2");

    settingsHolder.setTheme("BBB");
  }

  // Step 4: we do not use the journal file at the next restart
  {
    SettingsHolder settingsHolder;
    QVERIFY(!settingsHolder.recoveredFromJournal());
    QCOMPARE(settingsHolder.theme(), "BBB");
    QCOMPARE(settingsHolder.token(), "TOKEN 2");

    settingsHolder.setTheme("BBB");
  }
}

static TestSettings s_testSettings;
