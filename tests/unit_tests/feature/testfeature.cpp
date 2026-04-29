/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeature.h"

#include "feature/features.h"
#include "helper.h"
#include "settingsholder.h"

using namespace Feature;

void TestFeature::cleanup() { SettingsHolder::testCleanup(); }

void TestFeature::testConstantFeature() {
  constexpr ConstantFeature on = {
      .id = "testOn", .name = "On", .supported = true};
  constexpr ConstantFeature off = {
      .id = "testOff", .name = "Off", .supported = false};

  static_assert(isEnabled(on), "ConstantFeature(true) must be enabled");
  static_assert(!isEnabled(off), "ConstantFeature(false) must be disabled");

  QVERIFY(isEnabled(on));
  QVERIFY(!isEnabled(off));
}

void TestFeature::testRuntimeFeature() {
  static bool s_value = false;

  const RuntimeFeature f = {
      .id = "testRuntime",
      .name = "Runtime",
      .evaluator = +[] { return s_value; },
  };

  s_value = false;
  QVERIFY(!isEnabled(f));

  s_value = true;
  QVERIFY(isEnabled(f));

  s_value = false;
  QVERIFY(!isEnabled(f));
}

void TestFeature::testOverridableFeatureDefault() {
  const OverridableFeature on = {
      .id = "testOverridableOn",
      .name = "Overridable On",
      .evaluator = +[] { return true; },
  };

  const OverridableFeature off = {
      .id = "testOverridableOff",
      .name = "Overridable Off",
      .evaluator = +[] { return false; },
  };

  QVERIFY(isEnabled(on));
  QVERIFY(!isEnabled(off));
}

void TestFeature::testOverridableFeatureEvaluator() {
  static bool s_value = false;

  const OverridableFeature f = {
      .id = "testOverridableEval",
      .name = "Overridable Eval",
      .evaluator = +[] { return s_value; },
  };

  s_value = false;
  QVERIFY(!isEnabled(f));

  s_value = true;
  QVERIFY(isEnabled(f));
}

void TestFeature::testOverridableFeatureToggle() {
  auto* settings = SettingsHolder::instance();

  const OverridableFeature f = {
      .id = "testToggle",
      .name = "Toggle",
      .evaluator = +[] { return true; },
  };

  // Default: on
  QVERIFY(isEnabled(f));
  QVERIFY(!settings->featuresFlippedOn().contains("testToggle"));
  QVERIFY(!settings->featuresFlippedOff().contains("testToggle"));

  // Toggle off
  toggle(f, false);
  QVERIFY(!isEnabled(f));
  QVERIFY(settings->featuresFlippedOff().contains("testToggle"));

  // Toggle back on (restores default)
  toggle(f, true);
  QVERIFY(isEnabled(f));
  QVERIFY(!settings->featuresFlippedOff().contains("testToggle"));
  QVERIFY(!settings->featuresFlippedOn().contains("testToggle"));

  // Now test a default-off feature
  const OverridableFeature off = {
      .id = "testToggleOff",
      .name = "Toggle Off",
      .evaluator = +[] { return false; },
  };

  QVERIFY(!isEnabled(off));

  // Toggle on
  toggle(off, true);
  QVERIFY(isEnabled(off));
  QVERIFY(settings->featuresFlippedOn().contains("testToggleOff"));

  // Toggle back off (restores default)
  toggle(off, false);
  QVERIFY(!isEnabled(off));
  QVERIFY(!settings->featuresFlippedOn().contains("testToggleOff"));
  QVERIFY(!settings->featuresFlippedOff().contains("testToggleOff"));
}

static TestFeature s_testFeature;
