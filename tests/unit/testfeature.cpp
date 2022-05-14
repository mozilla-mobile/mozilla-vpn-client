/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeature.h"
#include "testfeaturelist.h"
#include "../../src/featurelist.h"
#include "../../src/features/featurecustomdns.h"
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
  FeatureTestA fA;
  QVERIFY(!!Feature::get("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  FeatureTestB fB;
  QVERIFY(!!Feature::get("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  FeatureTestC fC;
  QVERIFY(!!Feature::get("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  // FeatureList initialization
  FeatureList* fl = FeatureList::instance();
  fl->initialize();

  // Flipping on an already-supported feature doesn't produce changes.
  fl->toggleForcedEnable("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  // Flipping off a feature which was on -> it becomes off
  fl->toggleForcedDisable("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!Feature::get("testFeatureA")->isSupported());

  // Flipping off a feature which was on but disabled -> it becomes on
  fl->toggleForcedDisable("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  // Flipping off an non-supported feature doesn't produce changes.
  fl->toggleForcedDisable("testFeatureB");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  // Flipping on a feature which was off -> it becomes on
  fl->toggleForcedEnable("testFeatureB");
  QVERIFY(settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(Feature::get("testFeatureB")->isSupported());

  // Flipping on a feature which was off but enabled -> it becomes off
  fl->toggleForcedEnable("testFeatureB");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  // Flipping on an unflippable feature doesn't produce changes.
  fl->toggleForcedEnable("testFeatureC");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  // Flipping on an unflippable feature doesn't produce changes.
  fl->toggleForcedDisable("testFeatureC");
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
  // Let's create a few features
  FeatureTestA fA;
  QVERIFY(!!Feature::get("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  FeatureTestB fB;
  QVERIFY(!!Feature::get("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  FeatureTestC fC;
  QVERIFY(!!Feature::get("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  FeatureList::instance()->initialize();

  QJsonObject obj;
  obj["testFeatureA"] = true;
  obj["testFeatureB"] = true;
  obj["testFeatureC"] = true;

  QJsonObject json;
  json["featuresOverwrite"] = obj;

  FeatureList::instance()->updateFeatureList(QJsonDocument(json).toJson());
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

  FeatureList::instance()->updateFeatureList(QJsonDocument(json).toJson());
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

  FeatureList::instance()->updateFeatureList(QJsonDocument(json).toJson());
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
