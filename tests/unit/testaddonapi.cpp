/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddonapi.h"
#include "../../src/addons/addon.h"
#include "../../src/addons/addonmessage.h"
#include "../../src/addons/conditionwatchers/addonconditionwatcherjavascript.h"
#include "../../src/models/feature.h"
#include "../../src/qmlengineholder.h"
#include "../../src/settingsholder.h"
#include "helper.h"

#include <QQmlApplicationEngine>

void TestAddonApi::controller() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QVERIFY(!!message);

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_controller.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
}

void TestAddonApi::env() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QVERIFY(!!message);

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_env.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
}

void TestAddonApi::featurelist() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QVERIFY(!!message);

  QVERIFY(!Feature::getOrNull("testFeatureAddonApi"));
  Feature feature(
      "testFeatureAddonApi", "Feature Addon API",
      false,                          // Is Major Feature
      L18nStrings::Empty,             // Display name
      L18nStrings::Empty,             // Description
      L18nStrings::Empty,             // LongDescr
      "",                             // ImagePath
      "",                             // IconPath
      "",                             // link URL
      "1.0",                          // released
      []() -> bool { return true; },  // Can be flipped on
      []() -> bool { return true; },  // Can be flipped off
      QStringList(),                  // feature dependencies
      []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureAddonApi"));
  QVERIFY(!Feature::get("testFeatureAddonApi")->isSupported());

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_featurelist.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());

  QVERIFY(Feature::get("testFeatureAddonApi")->isSupported());
}

void TestAddonApi::navigator() {
  MozillaVPN vpn;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  SettingsHolder settingsHolder;

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QVERIFY(!!message);

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_navigator.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
}

void TestAddonApi::settings() {
  MozillaVPN vpn;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  SettingsHolder settingsHolder;

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QVERIFY(!!message);

  settingsHolder.setPostAuthenticationShown(false);
  QVERIFY(!settingsHolder.postAuthenticationShown());

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_settings.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
  QVERIFY(settingsHolder.postAuthenticationShown());
}

void TestAddonApi::urlopener() {
  MozillaVPN vpn;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  SettingsHolder settingsHolder;

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QVERIFY(!!message);

  settingsHolder.setPostAuthenticationShown(false);

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_urlopener.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
}

static TestAddonApi s_testAddonApi;
