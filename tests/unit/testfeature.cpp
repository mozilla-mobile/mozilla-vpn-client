/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testfeature.h"
#include "../../src/featurelist.h"
#include "../../src/features/featurecustomdns.h"
#include "../../src/models/feature.h"
#include "../../src/settingsholder.h"
#include "../../src/adjust/adjustfiltering.h"
#include "helper.h"

void TestFeature::enableByAPI() {
  SettingsHolder::instance();

  FeatureList::instance().initialize();

  const Feature* feature = Feature::get(FEATURE_CUSTOM_DNS);
  QVERIFY(feature->isSupported());

  QJsonObject obj;
  obj["customDNS"] = false;

  QJsonObject json;
  json["features"] = obj;

  FeatureList::instance().updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(feature->isSupported());

  obj["customDNS"] = true;
  json["features"] = obj;

  FeatureList::instance().updateFeatureList(QJsonDocument(json).toJson());
  QVERIFY(feature->isSupported());

  obj["allowParameters"] = QJsonArray{"allowTest"};

  QJsonObject deny;
  deny["denyTest"] = "test";
  obj["denyParameters"] = deny;

  QJsonObject mirror;
  mirror["mirrorTest"] = QJsonArray{"test", "testValue"};
  obj["mirrorParameters"] = mirror;

  json["adjustFields"] = obj;

  FeatureList::instance().updateFeatureList(QJsonDocument(json).toJson());
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
