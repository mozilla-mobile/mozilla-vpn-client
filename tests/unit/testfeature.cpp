/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeature.h"
#include "../../src/models/feature.h"
#include "../../src/models/featuremodel.h"
#include "../../src/models/feature.h"
#include "../../src/settingsholder.h"
#include "../../src/adjust/adjustfiltering.h"
#include "helper.h"

void TestFeature::flipOnOff() {
  SettingsHolder settingsHolder;

  // Fresh settings!
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));

  // Let's create a few features

  QVERIFY(!Feature::getOrNull("testFeatureA"));
  Feature fA("testFeatureA", "Feature A",
             false,               // Is Major Feature
             L18nStrings::Empty,  // Display name
             L18nStrings::Empty,  // Description
             L18nStrings::Empty,  // LongDescr
             "",                  // ImagePath
             "",                  // IconPath
             "",                  // link URL
             "1.0",               // released
             true,                // Can be flipped on
             true,                // Can be flipped off
             QStringList(),       // feature dependencies
             []() -> bool { return true; });
  QVERIFY(!!Feature::get("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  QVERIFY(!Feature::getOrNull("testFeatureB"));
  Feature fB("testFeatureB", "Feature B",
             false,               // Is Major Feature
             L18nStrings::Empty,  // Display name
             L18nStrings::Empty,  // Description
             L18nStrings::Empty,  // LongDescr
             "",                  // ImagePath
             "",                  // IconPath
             "",                  // link URL
             "1.0",               // released
             true,                // Can be flipped on
             true,                // Can be flipped off
             QStringList(),       // feature dependencies
             []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  QVERIFY(!Feature::getOrNull("testFeatureC"));
  Feature fC("testFeatureC", "Feature C",
             false,               // Is Major Feature
             L18nStrings::Empty,  // Display name
             L18nStrings::Empty,  // Description
             L18nStrings::Empty,  // LongDescr
             "",                  // ImagePath
             "",                  // IconPath
             "",                  // link URL
             "1.0",               // released
             false,               // Can be flipped on
             false,               // Can be flipped off
             QStringList(),       // feature dependencies
             []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  FeatureModel* fm = FeatureModel::instance();

  // Flipping on an already-supported feature doesn't produce changes.
  fm->toggleForcedEnable("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  // Flipping off a feature which was on -> it becomes off
  fm->toggleForcedDisable("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!Feature::get("testFeatureA")->isSupported());

  // Flipping off a feature which was on but disabled -> it becomes on
  fm->toggleForcedDisable("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  // Flipping off an non-supported feature doesn't produce changes.
  fm->toggleForcedDisable("testFeatureB");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  // Flipping on a feature which was off -> it becomes on
  fm->toggleForcedEnable("testFeatureB");
  QVERIFY(settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(Feature::get("testFeatureB")->isSupported());

  // Flipping on a feature which was off but enabled -> it becomes off
  fm->toggleForcedEnable("testFeatureB");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  // Flipping on an unflippable feature doesn't produce changes.
  fm->toggleForcedEnable("testFeatureC");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  // Flipping on an unflippable feature doesn't produce changes.
  fm->toggleForcedDisable("testFeatureC");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());
}

void TestFeature::enableByAPI() {
  SettingsHolder settingsHolder;
  // Fresh settings!
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));

  // Let's create a few features

  Feature fA("testFeatureA", "Feature A",
             false,               // Is Major Feature
             L18nStrings::Empty,  // Display name
             L18nStrings::Empty,  // Description
             L18nStrings::Empty,  // LongDescr
             "",                  // ImagePath
             "",                  // IconPath
             "",                  // link URL
             "1.0",               // released
             true,                // Can be flipped on
             true,                // Can be flipped off
             QStringList(),       // feature dependencies
             []() -> bool { return true; });
  QVERIFY(!!Feature::get("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  Feature fB("testFeatureB", "Feature B",
             false,               // Is Major Feature
             L18nStrings::Empty,  // Display name
             L18nStrings::Empty,  // Description
             L18nStrings::Empty,  // LongDescr
             "",                  // ImagePath
             "",                  // IconPath
             "",                  // link URL
             "1.0",               // released
             true,                // Can be flipped on
             true,                // Can be flipped off
             QStringList(),       // feature dependencies
             []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  Feature fC("testFeatureC", "Feature C",
             false,               // Is Major Feature
             L18nStrings::Empty,  // Display name
             L18nStrings::Empty,  // Description
             L18nStrings::Empty,  // LongDescr
             "",                  // ImagePath
             "",                  // IconPath
             "",                  // link URL
             "1.0",               // released
             false,               // Can be flipped on
             false,               // Can be flipped off
             QStringList(),       // feature dependencies
             []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  QJsonObject obj;
  obj["testFeatureA"] = true;
  obj["testFeatureB"] = true;
  obj["testFeatureC"] = true;

  QJsonObject json;
  json["featuresOverwrite"] = obj;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());
  QVERIFY(Feature::get("testFeatureB")->isSupported());
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  obj["testFeatureA"] = false;
  obj["testFeatureB"] = false;
  obj["testFeatureC"] = false;
  json["featuresOverwrite"] = obj;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureA")->isSupported());
  QVERIFY(!Feature::get("testFeatureB")->isSupported());
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  obj["allowParameters"] = QJsonArray{"allowTest"};

  QJsonObject deny;
  deny["denyTest"] = "test";
  obj["denyParameters"] = deny;

  QJsonObject mirror;
  mirror["mirrorTest"] = QJsonArray{"test", "testValue"};
  obj["mirrorParameters"] = mirror;

  json["adjustFields"] = obj;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(AdjustFiltering::instance()->allowList.contains("allowTest"));
  QVERIFY(AdjustFiltering::instance()->denyList.contains("denyTest"));
  QVERIFY(AdjustFiltering::instance()->denyList.value("denyTest") == "test");
  QVERIFY(AdjustFiltering::instance()->mirrorList.contains("mirrorTest"));
  QVERIFY(AdjustFiltering::instance()
              ->mirrorList.value("mirrorTest")
              .m_mirrorParamName == "test");
  QVERIFY(AdjustFiltering::instance()
              ->mirrorList.value("mirrorTest")
              .m_defaultValue == "testValue");
}

static TestFeature s_testFeature;
