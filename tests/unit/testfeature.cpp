/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeature.h"
#include "../../src/featurelist.h"
#include "../../src/features/featuresharelogs.h"
#include "../../src/models/feature.h"
#include "../../src/settingsholder.h"
#include "helper.h"

void TestFeature::enableByAPI() {
  SettingsHolder settingsHolder;

  FeatureList::instance()->initialize();

  const Feature* feature = Feature::get(FEATURE_SHARE_LOGS);
  QVERIFY(!feature->isSupported());

  QJsonObject obj;
  obj["shareLogs"] = true;

  QJsonObject json;
  json["features"] = obj;

  FeatureList::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(feature->isSupported());

  obj["shareLogs"] = false;
  json["features"] = obj;

  FeatureList::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(!feature->isSupported());
}

static TestFeature s_testFeature;
