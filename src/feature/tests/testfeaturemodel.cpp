/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeaturemodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "adjust/adjustfiltering.h"
#include "feature/feature.h"
#include "feature/featuremodel.h"
#include "settings/settingsholder.h"

namespace {
SettingsHolder* s_settingsHolder = nullptr;
}

void TestFeatureModel::init() {
  s_settingsHolder = new SettingsHolder();

  Feature::maybeInitialize();
}

void TestFeatureModel::cleanup() {
  delete s_settingsHolder;
  Feature::testReset();
}

void TestFeatureModel::flipOnOff() {
  // Fresh settings!
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureC"));

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
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(s_settingsHolder->featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!Feature::get("testFeatureA")->isSupported());

  // On(+flip off) -> On
  fm->toggle("testFeatureA");
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());

  // Off -> Off(+flip on)
  fm->toggle("testFeatureB");
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(Feature::get("testFeatureB")->isSupported());

  // Off(+flip on) -> Off
  fm->toggle("testFeatureB");
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!Feature::get("testFeatureB")->isSupported());

  // Flipping on an unflippable feature doesn't produce changes.
  fm->toggle("testFeatureC");
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureC")->isSupported());
}

void TestFeatureModel::updateFeatureListOverwriteFeatures() {
  // Fresh settings
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureC"));

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
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(Feature::get("testFeatureA")->isSupported());
  QVERIFY(Feature::get("testFeatureB")->isSupported());
  QVERIFY(!Feature::get("testFeatureC")->isSupported());

  obj["testFeatureA"] = false;
  obj["testFeatureB"] = false;
  obj["testFeatureC"] = false;
  json["featuresOverwrite"] = obj;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureA"));
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOn().contains("testFeatureC"));
  QVERIFY(s_settingsHolder->featuresFlippedOff().contains("testFeatureA"));
  QVERIFY(s_settingsHolder->featuresFlippedOff().contains("testFeatureB"));
  QVERIFY(!s_settingsHolder->featuresFlippedOff().contains("testFeatureC"));
  QVERIFY(!Feature::get("testFeatureA")->isSupported());
  QVERIFY(!Feature::get("testFeatureB")->isSupported());
  QVERIFY(!Feature::get("testFeatureC")->isSupported());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesEmpty() {
  // Clean up the features state.
  s_settingsHolder->setFeaturesFlippedOn({});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

  QJsonObject obj;
  // Empty object ¯\_(ツ)_/¯
  QJsonObject experimentalFeatures;
  obj["experimentalFeatures"] = experimentalFeatures;
  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got an empty object
  QCOMPARE(initialFeaturesOnState.count(),
           s_settingsHolder->featuresFlippedOn().count());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesNotObject() {
  // Clean up the features state.
  s_settingsHolder->setFeaturesFlippedOn({});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

  QJsonObject obj;
  obj["experimentalFeatures"] = "notanobject";
  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got misformatted input
  QCOMPARE(initialFeaturesOnState.count(),
           s_settingsHolder->featuresFlippedOn().count());
}

void TestFeatureModel::
    updateFeatureListExperimentalFeaturesUnknownExperiment() {
  // Clean up the features state.
  s_settingsHolder->setFeaturesFlippedOn({});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  QJsonObject unknownExperiment;
  experimentalFeatures["unknownExperiment"] = unknownExperiment;
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got an unknown feature
  QCOMPARE(initialFeaturesOnState.count(),
           s_settingsHolder->featuresFlippedOn().count());
}

void TestFeatureModel::
    updateFeatureListExperimentalFeaturesNonObjectExperiment() {
  // Clean up the features state.
  s_settingsHolder->setFeaturesFlippedOn({});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  experimentalFeatures["myExperimentalFeature"] = "notanobject";
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Nothing happened, because we got a misconfigured feature
  QCOMPARE(initialFeaturesOnState.count(),
           s_settingsHolder->featuresFlippedOn().count());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesNoSettings() {
  // Clean up the features state.
  s_settingsHolder->setFeaturesFlippedOn({});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  QJsonObject myExperimentalFeature;
  experimentalFeatures["myExperimentalFeature"] = myExperimentalFeature;
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // Finally, it is flipped on!
  QCOMPARE(initialFeaturesOnState.count() + 1,
           s_settingsHolder->featuresFlippedOn().count());
  QVERIFY(
      s_settingsHolder->featuresFlippedOn().contains("myExperimentalFeature"));

  // No settings were set though, settings were empty.
  QVERIFY(!s_settingsHolder->myExperimentalFeature()->get("one").isValid());
  QVERIFY(!s_settingsHolder->myExperimentalFeature()->get("two").isValid());
  QVERIFY(!s_settingsHolder->myExperimentalFeature()->get("three").isValid());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesInvalidSettings() {
  // Clean up the features state.
  s_settingsHolder->setFeaturesFlippedOn({});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

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
           s_settingsHolder->featuresFlippedOn().count());
  QVERIFY(
      s_settingsHolder->featuresFlippedOn().contains("myExperimentalFeature"));

  // No settings were set though, settings had invalid values
  QVERIFY(!s_settingsHolder->myExperimentalFeature()->get("one").isValid());
  QVERIFY(!s_settingsHolder->myExperimentalFeature()->get("two").isValid());
  QVERIFY(!s_settingsHolder->myExperimentalFeature()->get("three").isValid());
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesValidSettings() {
  // Clean up the features state.
  s_settingsHolder->setFeaturesFlippedOn({});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

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
           s_settingsHolder->featuresFlippedOn().count());
  QVERIFY(
      s_settingsHolder->featuresFlippedOn().contains("myExperimentalFeature"));

  // And finally, the settings were set!
  QCOMPARE(s_settingsHolder->myExperimentalFeature()->get("one").toString(),
           "one");
  QCOMPARE(s_settingsHolder->myExperimentalFeature()->get("two").toString(),
           "two");
  QCOMPARE(s_settingsHolder->myExperimentalFeature()->get("three").toString(),
           "three");
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesToggleOnOff() {
  // Setup the initial feature state with some random features that shouldn't be
  // messed with by meddling with the experimental features.
  s_settingsHolder->setFeaturesFlippedOn({"some", "random", "feature"});

  auto initialFeaturesOnState = s_settingsHolder->featuresFlippedOn();

  QJsonObject obj;
  QJsonObject experimentalFeatures;
  QJsonObject myExperimentalFeature;
  experimentalFeatures["myExperimentalFeature"] = myExperimentalFeature;
  obj["experimentalFeatures"] = experimentalFeatures;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // It is flipped on!
  QCOMPARE(initialFeaturesOnState.count() + 1,
           s_settingsHolder->featuresFlippedOn().count());
  QVERIFY(
      s_settingsHolder->featuresFlippedOn().contains("myExperimentalFeature"));
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("some"));
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("random"));
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("feature"));

  QJsonObject emptyObj;
  obj["experimentalFeatures"] = emptyObj;
  FeatureModel::instance()->updateFeatureList(QJsonDocument(obj).toJson());

  // It is flipped off!
  QCOMPARE(initialFeaturesOnState.count(),
           s_settingsHolder->featuresFlippedOn().count());
  QVERIFY(
      !s_settingsHolder->featuresFlippedOn().contains("myExperimentalFeature"));
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("some"));
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("random"));
  QVERIFY(s_settingsHolder->featuresFlippedOn().contains("feature"));
}

QTEST_MAIN(TestFeatureModel);
