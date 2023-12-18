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
    auto group = SettingGroup("aGroup");

    // Let's add some keys to they group.
    group.set("keyOne", QVariant(1));
    group.set("keyTwo", QVariant(2));
    group.set("keyThree", QVariant(3));
  }

  auto group = SettingGroup("aGroup");

  // Check that just by calling the initializer the settings were registered.
  auto settingOne = SettingsManager::getSetting("aGroup/keyOne");
  Q_ASSERT(settingOne);
  auto settingTwo = SettingsManager::getSetting("aGroup/keyTwo");
  Q_ASSERT(settingTwo);
  auto settingThree = SettingsManager::getSetting("aGroup/keyThree");
  Q_ASSERT(settingThree);
}

void TestSettingGroup::testSettingsAreCreatedWithTheRightProperties() {
  bool expectedRemoveWhenReset = false;
  bool expectedSensitiveSetting = true;

  auto group =
      SettingGroup("aGroup", expectedRemoveWhenReset, expectedSensitiveSetting);

  // Set something so the setting is created.
  group.set("keyOne", QVariant(1));

  auto setting = SettingsManager::getSetting("aGroup/keyOne");
  QCOMPARE(setting->m_removeWhenReset, expectedRemoveWhenReset);
  QCOMPARE(setting->m_sensitiveSetting, expectedSensitiveSetting);
}

void TestSettingGroup::testGetAndSetUnregisteredSetting() {
  auto group = SettingGroup("aGroup");

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
  auto group = SettingGroup("aGroup", true, false, QStringList("allowedKey"));

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
  auto group = SettingGroup("aGroup", true, false, QStringList("allowedKey"));

  // Let's add some keys to they group.
  group.set("keyOne", QVariant(1));
  group.set("keyTwo", QVariant(2));
  group.set("keyThree", QVariant(3));

  // Not let's clean the whole group;
  group.remove();

  QVERIFY(group.get("keyOne").isNull());
  QVERIFY(group.get("keyTwo").isNull());
  QVERIFY(group.get("keyThree").isNull());

  // Over zealously check that the settings were also unregisterd.
  auto settingOne = SettingsManager::getSetting("aGroup/keyOne");
  Q_ASSERT(!settingOne);
  auto settingTwo = SettingsManager::getSetting("aGroup/keyTwo");
  Q_ASSERT(!settingTwo);
  auto settingThree = SettingsManager::getSetting("aGroup/keyThree");
  Q_ASSERT(!settingThree);
}

static TestSettingGroup s_testSettingGroup;
