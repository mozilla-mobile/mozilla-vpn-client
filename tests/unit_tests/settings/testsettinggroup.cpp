/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsettinggroup.h"

#include "helper.h"
#include "settings/setting.h"
#include "settings/settinggroup.h"
#include "settings/settingsmanager.h"

void TestSettingGroup::cleanup() {
  // This test suite requires a manual clean up,
  // because SettingsManager clean up is tied to the lifetime of a
  // SettingsHolder object and this test suite doesn't rely on the
  // SettingsHolder at all.
  SettingsManager::testCleanup();
}

void TestSettingGroup::testLoadSettingsOnInit() {
  {
    auto group = SettingsManager::createSettingGroup("aGroup");

    // Let's add some keys to they group.
    group.set("keyOne", QVariant(1));
    group.set("keyTwo", QVariant(2));
    group.set("keyThree", QVariant(3));
  }

  auto group = SettingsManager::createSettingGroup("aGroup");

  // Check that just by calling the initializer the settings were registered.
  auto settingOne = SettingsManager::instance()->getSetting("aGroup/keyOne");
  Q_ASSERT(settingOne);
  auto settingTwo = SettingsManager::instance()->getSetting("aGroup/keyTwo");
  Q_ASSERT(settingTwo);
  auto settingThree =
      SettingsManager::instance()->getSetting("aGroup/keyThree");
  Q_ASSERT(settingThree);
}

void TestSettingGroup::testSettingsAreCreatedWithTheRightProperties() {
  bool expectedRemoveWhenReset = false;
  bool expectedSensitiveSetting = true;

  auto group = SettingsManager::createSettingGroup(
      "aGroup", expectedRemoveWhenReset, expectedSensitiveSetting);

  // Set something so the setting is created.
  group.set("keyOne", QVariant(1));

  auto setting = SettingsManager::instance()->getSetting("aGroup/keyOne");
  QCOMPARE(setting->m_removeWhenReset, expectedRemoveWhenReset);
  QCOMPARE(setting->m_sensitiveSetting, expectedSensitiveSetting);
}

void TestSettingGroup::testGetAndSetUnregisteredSetting() {
  auto group = SettingsManager::createSettingGroup("aGroup");

  QSignalSpy spy(&group, &SettingGroup::changed);

  group.set("keyOne", QVariant(1));

  // The setting was unregistered,
  // but setting registered and stored a value in it.
  QCOMPARE(1, group.get("keyOne").toInt());

  QCOMPARE(spy.count(), 1);

  // Set again, why not.
  group.set("keyOne", QVariant(10));
  // Check the value did change.
  QCOMPARE(10, group.get("keyOne").toInt());

  QCOMPARE(spy.count(), 2);
}

void TestSettingGroup::testGetAndSetDisallowedSetting() {
  auto group = SettingsManager::createSettingGroup("aGroup", true, false,
                                                   QStringList("allowedKey"));

  QSignalSpy spy(&group, &SettingGroup::changed);

  group.set("keyOne", QVariant(1));
  QVERIFY(group.get("keyOne").isNull());

  // No signal, nothing changed. "keyOne" is not allowed.
  QCOMPARE(spy.count(), 0);

  group.set("allowedKey", QVariant("I AM ALLOWED!"));
  QCOMPARE("I AM ALLOWED!", group.get("allowedKey").toString());

  QCOMPARE(spy.count(), 1);
}

void TestSettingGroup::testRemove() {
  // Control setting to make sure only settings in the group are removed.
  auto notInTheGroup = SettingsManager::createOrGetSetting("notinthegroup");
  notInTheGroup->set(QVariant("aha!"));

  auto group = SettingsManager::createSettingGroup("aGroup", true, false);

  // Let's add some settings to they group.
  group.set("keyOne", QVariant(1));
  group.set("keyTwo", QVariant(2));
  group.set("keyThree", QVariant(3));

  QVERIFY(!notInTheGroup->get().isNull());
  QVERIFY(!group.get("keyOne").isNull());
  QVERIFY(!group.get("keyTwo").isNull());
  QVERIFY(!group.get("keyThree").isNull());

  // Not let's clean the whole group;
  group.remove();

  QVERIFY(group.get("keyOne").isNull());
  QVERIFY(group.get("keyTwo").isNull());
  QVERIFY(group.get("keyThree").isNull());
  // The setting not in the group is still there though.
  QVERIFY(!notInTheGroup->get().isNull());
}

static TestSettingGroup s_testSettingGroup;
