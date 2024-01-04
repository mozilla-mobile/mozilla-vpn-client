/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeaturemodel.h"

#include "adjust/adjustfiltering.h"
#include "feature/feature.h"
#include "feature/featuremodel.h"
#include "helper.h"
#include "settingsholder.h"

void TestFeatureModel::flipOnOff() {
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

  QVERIFY(!Feature::getOrNull("testFeatureC"));
  Feature fC(
      "testFeatureC", "Feature C",
      []() -> bool { return false; },  // Can be flipped on
      []() -> bool { return false; },  // Can be flipped off
      QStringList(),                   // feature dependencies
      []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  FeatureModel* fm = FeatureModel::instance();

  // On -> On(+flip off)
  fm->toggle("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!Feature::get("testFeatureA")->isSupported());

  // On(+flip off) -> On
  fm->toggle("testFeatureA");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  // Off -> Off(+flip on)
  fm->toggle("testFeatureB");
  QVERIFY(settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(Feature::get("testFeatureB")->isSupported());

  // Off(+flip on) -> Off
  fm->toggle("testFeatureB");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  // Flipping on an unflippable feature doesn't produce changes.
  fm->toggle("testFeatureC");
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());
}

void TestFeatureModel::updateFeatureListOverwriteFeatures() {
  SettingsHolder settingsHolder;
  // Fresh settings!
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!settingsHolder.featuresFlippedOff().contains("testFeatureC"));

  // Let's create a few features

  Feature fA(
      "testFeatureA", "Feature A",
      []() -> bool { return true; },  // Can be flipped on
      []() -> bool { return true; },  // Can be flipped off
      QStringList(),                  // feature dependencies
      []() -> bool { return true; });
  QVERIFY(!!Feature::get("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  Feature fB(
      "testFeatureB", "Feature B",
      []() -> bool { return true; },  // Can be flipped on
      []() -> bool { return true; },  // Can be flipped off
      QStringList(),                  // feature dependencies
      []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  Feature fC(
      "testFeatureC", "Feature C",
      []() -> bool { return false; },  // Can be flipped on
      []() -> bool { return false; },  // Can be flipped off
      QStringList(),                   // feature dependencies
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

void TestFeatureModel::updateFeatureListExperimentalFeaturesEmpty() {
  SettingsHolder settingsHolder;

  // Clean up the features state.
  settingsHolder.setFeaturesFlippedOn({});
  settingsHolder.setFeaturesFlippedOff({});

  auto initialFeaturesOnState = settingsHolder.featuresFlippedOn();
  auto initialFeaturesOffState = settingsHolder.featuresFlippedOff();

  QJsonObject obj;
  // Empty object ¯\_(ツ)_/¯
  QJsonObject experimentalFeatures;
  obj["experimentalFeatures"] = experimentalFeatures;
  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got an empty object
  QCOMPARE(initialFeaturesOnState.count(),
           settingsHolder.featuresFlippedOn().count());
  QCOMPARE(initialFeaturesOffState.count(),
           settingsHolder.featuresFlippedOff().count());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesNotObject() {
  SettingsHolder settingsHolder;

  // Clean up the features state.
  settingsHolder.setFeaturesFlippedOn({});
  settingsHolder.setFeaturesFlippedOff({});

  auto initialFeaturesOnState = settingsHolder.featuresFlippedOn();
  auto initialFeaturesOffState = settingsHolder.featuresFlippedOff();

  QJsonObject obj;
  obj["experimentalFeatures"] = "notanobject";
  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got misformatted input
  QCOMPARE(initialFeaturesOnState.count(),
           settingsHolder.featuresFlippedOn().count());
  QCOMPARE(initialFeaturesOffState.count(),
           settingsHolder.featuresFlippedOff().count());
}

void TestFeatureModel::
    updateFeatureListExperimentalFeaturesUnknownExperiment() {
  SettingsHolder settingsHolder;

  // Clean up the features state.
  settingsHolder.setFeaturesFlippedOn({});
  settingsHolder.setFeaturesFlippedOff({});

  auto initialFeaturesOnState = settingsHolder.featuresFlippedOn();
  auto initialFeaturesOffState = settingsHolder.featuresFlippedOff();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  QJsonObject unknownExperiment;
  experimentalFeatures["unknownExperiment"] = unknownExperiment;
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got an unknown feature
  QCOMPARE(initialFeaturesOnState.count(),
           settingsHolder.featuresFlippedOn().count());
  QCOMPARE(initialFeaturesOffState.count(),
           settingsHolder.featuresFlippedOff().count());
}

void TestFeatureModel::
    updateFeatureListExperimentalFeaturesNonObjectExperiment() {
  SettingsHolder settingsHolder;

  // Clean up the features state.
  settingsHolder.setFeaturesFlippedOn({});
  settingsHolder.setFeaturesFlippedOff({});

  auto initialFeaturesOnState = settingsHolder.featuresFlippedOn();
  auto initialFeaturesOffState = settingsHolder.featuresFlippedOff();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  experimentalFeatures["unknownExperiment"] = "notanobject";
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got a misconfigured feature
  QCOMPARE(initialFeaturesOnState.count(),
           settingsHolder.featuresFlippedOn().count());
  QCOMPARE(initialFeaturesOffState.count(),
           settingsHolder.featuresFlippedOff().count());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesNoSettings() {
  SettingsHolder settingsHolder;

  // Clean up the features state.
  settingsHolder.setFeaturesFlippedOn({});
  settingsHolder.setFeaturesFlippedOff({});

  auto initialFeaturesOnState = settingsHolder.featuresFlippedOn();
  auto initialFeaturesOffState = settingsHolder.featuresFlippedOff();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  QJsonObject myExperimentalFeature;
  experimentalFeatures["myExperimentalFeature"] = myExperimentalFeature;
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Finally, it is flipped on!
  QCOMPARE(initialFeaturesOnState.count() + 1,
           settingsHolder.featuresFlippedOn().count());
  QVERIFY(settingsHolder.featuresFlippedOn().contains("myExperimentalFeature"));
  QCOMPARE(initialFeaturesOffState.count(),
           settingsHolder.featuresFlippedOff().count());

  // No settings were set though, settings were empty.
  QVERIFY(!settingsHolder.myExperimentalFeature()->get("one").isValid());
  QVERIFY(!settingsHolder.myExperimentalFeature()->get("two").isValid());
  QVERIFY(!settingsHolder.myExperimentalFeature()->get("three").isValid());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesInvalidSettings() {
  SettingsHolder settingsHolder;

  // Clean up the features state.
  settingsHolder.setFeaturesFlippedOn({});
  settingsHolder.setFeaturesFlippedOff({});

  auto initialFeaturesOnState = settingsHolder.featuresFlippedOn();
  auto initialFeaturesOffState = settingsHolder.featuresFlippedOff();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  QJsonObject myExperimentalFeature;
  myExperimentalFeature["one"] = QJsonValue::Null;
  myExperimentalFeature["two"] = QJsonValue::Null;
  myExperimentalFeature["three"] = QJsonValue::Null;
  experimentalFeatures["myExperimentalFeature"] = myExperimentalFeature;
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // It is flipped on!
  QCOMPARE(initialFeaturesOnState.count() + 1,
           settingsHolder.featuresFlippedOn().count());
  QVERIFY(settingsHolder.featuresFlippedOn().contains("myExperimentalFeature"));

  // No settings were set though, settings had invalid values
  QVERIFY(!settingsHolder.myExperimentalFeature()->get("one").isValid());
  QVERIFY(!settingsHolder.myExperimentalFeature()->get("two").isValid());
  QVERIFY(!settingsHolder.myExperimentalFeature()->get("three").isValid());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesValidSettings() {
  SettingsHolder settingsHolder;

  // Clean up the features state.
  settingsHolder.setFeaturesFlippedOn({});
  settingsHolder.setFeaturesFlippedOff({});

  auto initialFeaturesOnState = settingsHolder.featuresFlippedOn();
  auto initialFeaturesOffState = settingsHolder.featuresFlippedOff();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  QJsonObject myExperimentalFeature;
  myExperimentalFeature["one"] = "one";
  myExperimentalFeature["two"] = "two";
  myExperimentalFeature["three"] = "three";
  experimentalFeatures["myExperimentalFeature"] = myExperimentalFeature;
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // It is flipped on!
  QCOMPARE(initialFeaturesOnState.count() + 1,
           settingsHolder.featuresFlippedOn().count());
  QVERIFY(settingsHolder.featuresFlippedOn().contains("myExperimentalFeature"));

  // And finally, the settings were set!
  QCOMPARE(settingsHolder.myExperimentalFeature()->get("one").toString(),
           "one");
  QCOMPARE(settingsHolder.myExperimentalFeature()->get("two").toString(),
           "two");
  QCOMPARE(settingsHolder.myExperimentalFeature()->get("three").toString(),
           "three");
}

static TestFeatureModel s_testFeature;
