/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeaturemodel.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "feature/featuremodel.h"
#include "feature/features.h"
#include "helper.h"
#include "settingsholder.h"

using namespace Feature;

void TestFeatureModel::cleanup() { SettingsHolder::testCleanup(); }

void TestFeatureModel::flipOnOff() {
  auto* settings = SettingsHolder::instance();

  // alwaysPort53: default off, flippable
  QVERIFY(!isEnabled(alwaysPort53));

  // Toggle on
  FeatureModel::instance()->toggle("alwaysPort53");
  QVERIFY(isEnabled(alwaysPort53));
  QVERIFY(settings->featuresFlippedOn().contains("alwaysPort53"));

  // Toggle back off
  FeatureModel::instance()->toggle("alwaysPort53");
  QVERIFY(!isEnabled(alwaysPort53));
  QVERIFY(!settings->featuresFlippedOn().contains("alwaysPort53"));

  // recommendedServers: default on, flippable
  QVERIFY(isEnabled(recommendedServers));

  // Toggle off
  FeatureModel::instance()->toggle("recommendedServers");
  QVERIFY(!isEnabled(recommendedServers));
  QVERIFY(settings->featuresFlippedOff().contains("recommendedServers"));

  // Toggle back on
  FeatureModel::instance()->toggle("recommendedServers");
  QVERIFY(isEnabled(recommendedServers));
  QVERIFY(!settings->featuresFlippedOff().contains("recommendedServers"));
}

void TestFeatureModel::updateFeatureListOverwriteFeatures() {
  // alwaysPort53 is off by default, flippable
  QVERIFY(!isEnabled(alwaysPort53));

  // Server says: flip on
  QJsonObject overwrite;
  overwrite["alwaysPort53"] = true;
  QJsonObject json;
  json["featuresOverwrite"] = overwrite;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(isEnabled(alwaysPort53));

  // Server says: flip off
  overwrite["alwaysPort53"] = false;
  json["featuresOverwrite"] = overwrite;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(!isEnabled(alwaysPort53));
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesEmpty() {
  auto* settings = SettingsHolder::instance();
  settings->setFeaturesFlippedOn({});
  auto initialCount = settings->featuresFlippedOn().count();

  QJsonObject json;
  json["experimentalFeatures"] = QJsonObject();

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());

  // Empty object — nothing changed
  QCOMPARE(settings->featuresFlippedOn().count(), initialCount);
}

void TestFeatureModel::updateFeatureListExperimentalFeaturesNotObject() {
  auto* settings = SettingsHolder::instance();
  settings->setFeaturesFlippedOn({});
  auto initialCount = settings->featuresFlippedOn().count();

  QJsonObject json;
  json["experimentalFeatures"] = "notanobject";

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());

  // Malformed input — nothing changed
  QCOMPARE(settings->featuresFlippedOn().count(), initialCount);
}

void TestFeatureModel::
    updateFeatureListExperimentalFeaturesUnknownExperiment() {
  auto* settings = SettingsHolder::instance();
  settings->setFeaturesFlippedOn({});
  auto initialCount = settings->featuresFlippedOn().count();

  QJsonObject experimental;
  experimental["unknownExperiment"] = QJsonObject();
  QJsonObject json;
  json["experimentalFeatures"] = experimental;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());

  // Unknown feature — nothing changed
  QCOMPARE(settings->featuresFlippedOn().count(), initialCount);
}

void TestFeatureModel::
    updateFeatureListExperimentalFeaturesNonObjectExperiment() {
  auto* settings = SettingsHolder::instance();
  settings->setFeaturesFlippedOn({});
  auto initialCount = settings->featuresFlippedOn().count();

  QJsonObject experimental;
  experimental["alwaysPort53"] = "notanobject";
  QJsonObject json;
  json["experimentalFeatures"] = experimental;

  FeatureModel::instance()->updateFeatureList(QJsonDocument(json).toJson());

  // Malformed experiment — nothing changed
  QCOMPARE(settings->featuresFlippedOn().count(), initialCount);
}

static TestFeatureModel s_testFeatureModel;
