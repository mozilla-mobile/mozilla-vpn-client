/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeature.h"

#include "feature.h"
#include "helper.h"
#include "settingsholder.h"

void TestFeature::flipOnOff() {
  SettingsHolder settingsHolder;

  // Fresh settings!
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));

  // Let's create a few features

  QVERIFY(!Feature::getOrNull("testFeatureA"));
  Feature fA(
      "testFeatureA", "Feature A",
      []() -> bool { return true; },  // Can be flipped on
      []() -> bool { return true; },  // Can be flipped off
      QStringList(),                  // feature dependencies
      []() -> bool { return true; });
  QVERIFY(!!Feature::get("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  QVERIFY(!Feature::getOrNull("testFeatureB"));
  Feature fB(
      "testFeatureB", "Feature B",
      []() -> bool { return true; },  // Can be flipped on
      []() -> bool { return true; },  // Can be flipped off
      QStringList(),                  // feature dependencies
      []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());
}

static TestFeature s_testFeature;
