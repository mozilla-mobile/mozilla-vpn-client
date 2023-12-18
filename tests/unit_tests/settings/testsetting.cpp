/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsetting.h"

#include "helper.h"
#include "settings/setting.h"
#include "settings/settingfactory.h"
#include "settings/settingsmanager.h"

void TestSetting::cleanup() {
  // This test suite requires a manual clean up,
  // because SettingsManager clean up is tied to the lifetime of a
  // SettingsHolder object and this test suite doesn't rely on the
  // SettingsHolder at all.
  SettingsManager::testCleanup();
}

void TestSetting::testSetSameAsStoredValue() {
  auto oneTimeSet = SettingFactory::createOrGetSetting("yoso");

  QSignalSpy spy(oneTimeSet, &Setting::changed);

  oneTimeSet->set(QVariant("youonlysetonce"));
  QCOMPARE(spy.count(), 1);

  QCOMPARE(oneTimeSet->isSet(), true);
  QCOMPARE(oneTimeSet->get().toString(), "youonlysetonce");

  // Try as many times as you want, you only set once --
  // if trying to set to the same value.
  oneTimeSet->set(QVariant("youonlysetonce"));
  oneTimeSet->set(QVariant("youonlysetonce"));
  oneTimeSet->set(QVariant("youonlysetonce"));
  oneTimeSet->set(QVariant("youonlysetonce"));
  oneTimeSet->set(QVariant("youonlysetonce"));

  QCOMPARE(spy.count(), 1);

  // Now try to set to a different value and you are good to go.
  oneTimeSet->set(QVariant("youonlyliveonce"));
  QCOMPARE(spy.count(), 2);
  QCOMPARE(oneTimeSet->get().toString(), "youonlyliveonce");
}

// This is here to make sure the isSet check comes before the equality check
// against get() inside the set() function. That has bit me during development.
void TestSetting::testSetDefaultWhenUnset() {
  auto setToDefault = SettingFactory::createOrGetSetting(
      "justthedefault", []() { return QVariant("default"); }, false, false);

  QSignalSpy spy(setToDefault, &Setting::changed);

  setToDefault->set(QVariant("default"));
  QCOMPARE(spy.count(), 1);

  QCOMPARE(setToDefault->isSet(), true);
  QCOMPARE(setToDefault->get().toString(), "default");
}

void TestSetting::testIsSet() {
  auto justASetting = SettingFactory::createOrGetSetting("asciishrug");

  QCOMPARE(justASetting->isSet(), false);

  justASetting->set(QVariant("¯_(ツ)_/¯"));
  QCOMPARE(justASetting->isSet(), true);
}

void TestSetting::testRemove() {
  // Reset or not reset should not amke any difference for the `remove` API,
  // let's test both and verify that is true.
  auto doNotReset = SettingFactory::createOrGetSetting(
      "donotreset", []() { return nullptr; }, false, false);
  auto doReset = SettingFactory::createOrGetSetting(
      "doreset", []() { return nullptr; }, true, false);

  QSignalSpy doNotResetSpy(doNotReset, &Setting::changed);
  QSignalSpy doResetSpy(doReset, &Setting::changed);

  doNotReset->set(QVariant("never forget"));
  QCOMPARE(doNotResetSpy.count(), 1);
  QCOMPARE(doNotReset->get().toString(), "never forget");

  doNotReset->remove();
  // Not there anymore
  QCOMPARE(doNotReset->get().toString(), "");
  // Since stuff has changed, a signal was fired
  QCOMPARE(doNotResetSpy.count(), 2);
  // Since stuff has not changed yet, no signal was fired
  QCOMPARE(doResetSpy.count(), 0);

  doReset->set(QVariant("bye bye bye"));
  QCOMPARE(doResetSpy.count(), 1);
  QCOMPARE(doReset->get().toString(), "bye bye bye");

  doReset->reset();
  // Not there anymore either
  QCOMPARE(doReset->get().toString(), "");
  // Nothing else changed here, no extra signal
  QCOMPARE(doNotResetSpy.count(), 2);
  // This one has now changed, signal fired
  QCOMPARE(doResetSpy.count(), 2);
}

void TestSetting::testResetOnNotRemoveWhenReset() {
  auto doNotReset = SettingFactory::createOrGetSetting(
      "donotreset", []() { return nullptr; }, false, false);

  QSignalSpy spy(doNotReset, &Setting::changed);

  doNotReset->set(QVariant("never forget"));
  QCOMPARE(spy.count(), 1);
  QCOMPARE(doNotReset->get().toString(), "never forget");

  doNotReset->reset();
  // Still there
  QCOMPARE(doNotReset->get().toString(), "never forget");
  // Since nothing changed a signal was not fired
  QCOMPARE(spy.count(), 1);
}

void TestSetting::testResetOnRemoveWhenReset() {
  auto doReset = SettingFactory::createOrGetSetting(
      "doreset", []() { return nullptr; }, true, false);

  QSignalSpy spy(doReset, &Setting::changed);

  doReset->set(QVariant("bye bye bye"));
  QCOMPARE(spy.count(), 1);
  QCOMPARE(doReset->get().toString(), "bye bye bye");

  doReset->reset();
  // Not there anymore
  QCOMPARE(doReset->get().toString(), "");
  QCOMPARE(spy.count(), 2);
}

void TestSetting::testLogSensitiveSetting() {
  auto sensitiveSetting = SettingFactory::createOrGetSetting(
      "sensitive", []() { return nullptr; }, true, true);

  sensitiveSetting->set(QVariant("super sensitive text"));
  QCOMPARE(sensitiveSetting->log(), "sensitive -> <Sensitive>");
}

void TestSetting::testLogNotSensitiveSetting() {
  auto notSensitiveetting = SettingFactory::createOrGetSetting(
      "notsensitive", []() { return nullptr; }, true, false);

  notSensitiveetting->set(QVariant("public text"));
  QCOMPARE(notSensitiveetting->log(), "notsensitive -> public text");
}

void TestSetting::testLogEmptySetting() {
  auto notSensitiveetting = SettingFactory::createOrGetSetting(
      "notsensitive", []() { return nullptr; }, true, false);

  QCOMPARE(notSensitiveetting->log(), "");

  auto sensitiveSetting = SettingFactory::createOrGetSetting(
      "sensitive", []() { return nullptr; }, true, true);

  QCOMPARE(sensitiveSetting->log(), "");
}

static TestSetting s_testSetting;
