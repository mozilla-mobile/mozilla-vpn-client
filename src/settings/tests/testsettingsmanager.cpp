/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsettingsmanager.h"

#include <QSignalSpy>
#include <QTest>

#include "setting.h"
#include "settingsmanager.h"

void TestSettingsManager::cleanup() {
  // This test suite requires a manual clean up,
  // because SettingsManager clean up is tied to the lifetime of a
  // SettingsHolder object and this test suite doesn't rely on the
  // SettingsHolder at all.
  SettingsManager::testCleanup();
}

void TestSettingsManager::testGetSetting() {
  // Try to get a setting that doesn't exist. All good, but nullptr
  Q_ASSERT(!SettingsManager::instance()->getSetting("doesnotexist"));

  // Create a setting
  auto setting = SettingsManager::instance()->createOrGetSetting("doesexist");
  Q_UNUSED(setting)

  // Try to get it, now it's not a nullptr.
  Q_ASSERT(SettingsManager::instance()->getSetting("doesexist"));
}

void TestSettingsManager::testReset() {
  // Create a setting that should be reset
  auto doReset = SettingsManager::instance()->createOrGetSetting(
      "doreset", []() { return QVariant(); },
      true,  // remove when reset
      false  // sensitive setting
  );
  // Create a setting that should not be reset
  auto doNotReset = SettingsManager::instance()->createOrGetSetting(
      "donotreset", []() { return QVariant(); },
      false,  // remove when reset
      false   // sensitive setting
  );

  // Check they are there, just in case.
  Q_ASSERT(SettingsManager::instance()->getSetting("doreset"));
  Q_ASSERT(SettingsManager::instance()->getSetting("donotreset"));

  doReset->set(QVariant("hey"));
  doNotReset->set(QVariant("ho"));

  // Check that the values were recorded, just in case.
  QCOMPARE(doReset->get().toString(), "hey");
  QCOMPARE(doNotReset->get().toString(), "ho");

  // Now reset!
  SettingsManager::instance()->reset();

  // Reset should _not_ unregister. Check that is true.
  Q_ASSERT(SettingsManager::instance()->getSetting("doreset"));
  Q_ASSERT(SettingsManager::instance()->getSetting("donotreset"));

  // But it will clear the storage, if the setting is setup to do that.
  QVERIFY(doReset->get().isNull());
  QCOMPARE(doNotReset->get().toString(), "ho");
}

void TestSettingsManager::testHardReset() {
  // Create a setting that should be reset
  auto doReset = SettingsManager::instance()->createOrGetSetting(
      "doreset", []() { return QVariant(); },
      true,  // remove when reset
      false  // sensitive setting
  );
  // Create a setting that should not be reset... It doesn't really matter here,
  // everyone will get reset. But just in case.
  auto doNotReset = SettingsManager::instance()->createOrGetSetting(
      "donotreset", []() { return QVariant(); },
      false,  // remove when reset
      false   // sensitive setting
  );

  // Check they are there, just in case.
  Q_ASSERT(SettingsManager::instance()->getSetting("doreset"));
  Q_ASSERT(SettingsManager::instance()->getSetting("donotreset"));

  doReset->set(QVariant("hey"));
  doNotReset->set(QVariant("ho"));

  // Check that the values were recorded, just in case.
  QCOMPARE(doReset->get().toString(), "hey");
  QCOMPARE(doNotReset->get().toString(), "ho");

  QSignalSpy doResetSpy(doReset, &Setting::changed);
  QSignalSpy doNotResetSpy(doNotReset, &Setting::changed);

  // Now reset!
  SettingsManager::instance()->hardReset();

  // Hard reset will also not unregister settings. Check it.
  Q_ASSERT(SettingsManager::instance()->getSetting("doreset"));
  Q_ASSERT(SettingsManager::instance()->getSetting("donotreset"));

  // It will also clear the storage, for all settings regardless of
  // configuration.
  QVERIFY(doReset->get().isNull());
  QVERIFY(doNotReset->get().isNull());

  // On hard reset, the settings base triggers the change signals itself,
  // so we check.
  QCOMPARE(doResetSpy.count(), 1);
  QCOMPARE(doNotResetSpy.count(), 1);
}

void TestSettingsManager::testSerializeLogs() {
  // Register some settings.
  auto oneSetting = SettingsManager::instance()->createOrGetSetting("one");
  auto twoSetting = SettingsManager::instance()->createOrGetSetting("two");
  auto threeSetting = SettingsManager::instance()->createOrGetSetting("three");
  auto thisOneWeWillNotSet =
      SettingsManager::instance()->createOrGetSetting("neverset");
  Q_UNUSED(thisOneWeWillNotSet)

  oneSetting->set(QVariant(1));
  twoSetting->set(QVariant(2));
  threeSetting->set(QVariant(3));

  QString report;
  // We can do this because serializeLogs() is sync.
  SettingsManager::instance()->serializeLogs(
      [report = &report](const QString& name, const QString& logs) {
        *report = logs;
      });

  QVERIFY(report.contains("one -> 1"));
  QVERIFY(report.contains("two -> 2"));
  QVERIFY(report.contains("three -> 3"));
  QVERIFY(!report.contains("neverset ->"));
}

void TestSettingsManager::testCreateNewSetting() {
  QString expectedKey = "aKey";
  QVariant expectedDefault = "aDefaultValue";
  bool expectedRemoveWhenReset = false;
  bool expectedSensitiveSetting = true;

  auto setting = SettingsManager::instance()->createOrGetSetting(
      expectedKey, [&expectedDefault]() { return QVariant(expectedDefault); },
      expectedRemoveWhenReset, expectedSensitiveSetting);

  auto registeredSetting = SettingsManager::instance()->getSetting(expectedKey);

  // The registered setting is exactly the same as the setting created.
  QCOMPARE(setting, registeredSetting);

  // Check that the registered setting has the expected properties.

  // Getting without setting would return the default value.
  QCOMPARE(registeredSetting->get(), expectedDefault);
  QCOMPARE(registeredSetting->m_removeWhenReset, expectedRemoveWhenReset);
  QCOMPARE(registeredSetting->m_sensitiveSetting, expectedSensitiveSetting);
}

void TestSettingsManager::testCreateNewSettingButSettingAlreadyExists() {
  auto oneSetting = SettingsManager::instance()->createOrGetSetting("aKey");
  auto twoSetting = SettingsManager::instance()->createOrGetSetting("aKey");

  // They are the same,
  // because the second time around it just got what was already created!
  QCOMPARE(oneSetting, twoSetting);
}

QTEST_MAIN(TestSettingsManager)
