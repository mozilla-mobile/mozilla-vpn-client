/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsettingfactory.h"

#include "helper.h"
#include "settings/setting.h"
#include "settings/settingfactory.h"
#include "settings/settingsmanager.h"

void TestSettingFactory::cleanup() {
  // This test suite requires a manual clean up,
  // because SettingsManager clean up is tied to the lifetime of a
  // SettingsHolder object and this test suite doesn't rely on the
  // SettingsHolder at all.
  SettingsManager::testCleanup();
}

void TestSettingFactory::testCreateNewSetting() {
  QString expectedKey = "aKey";
  QVariant expectedDefault = "aDefaultValue";
  bool expectedRemoveWhenReset = false;
  bool expectedSensitiveSetting = true;

  auto setting = SettingFactory::createOrGetSetting(
      expectedKey, [&expectedDefault]() { return QVariant(expectedDefault); },
      expectedRemoveWhenReset, expectedSensitiveSetting);

  auto registeredSetting = SettingsManager::getSetting(expectedKey);

  // The registered setting is exactly the same as the setting created.
  QCOMPARE(setting, registeredSetting);

  // Check that the registered setting has the expected properties.

  // Getting without setting would return the default value.
  QCOMPARE(registeredSetting->get(), expectedDefault);
  QCOMPARE(registeredSetting->m_removeWhenReset, expectedRemoveWhenReset);
  QCOMPARE(registeredSetting->m_sensitiveSetting, expectedSensitiveSetting);
}

void TestSettingFactory::testCreateNewSettingButSettingAlreadyExists() {
  auto oneSetting = SettingFactory::createOrGetSetting("aKey");
  auto twoSetting = SettingFactory::createOrGetSetting("aKey");

  // They are the same,
  // because the second time around it just got what was already created!
  QCOMPARE(oneSetting, twoSetting);
}

static TestSettingFactory s_testSettingFactory;
