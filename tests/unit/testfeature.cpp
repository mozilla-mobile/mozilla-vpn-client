/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeature.h"
#include "../../src/featurelist.h"
#include "../../src/features/featureunauthsupport.h"
#include "../../src/models/feature.h"
#include "../../src/settingsholder.h"
#include "helper.h"

void TestFeature::enableByAPI() {
  SettingsHolder settingsHolder;

  FeatureList::instance()->initialize();

  const Feature* feature = Feature::get(FEATURE_UNAUTH_SUPPORT);
  QVERIFY(!feature->isSupported());

  QJsonObject json;
  json["unauthSupport"] = true;

  FeatureList::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(feature->isSupported());

  json["unauthSupport"] = false;

  FeatureList::instance()->updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(!feature->isSupported());
}

static TestFeature s_testFeature;
