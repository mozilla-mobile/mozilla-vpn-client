/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddonapi.h"

#include <QQmlApplicationEngine>

#include "addons/addon.h"
#include "addons/addonapi.h"
#include "addons/addonmessage.h"
#include "addons/conditionwatchers/addonconditionwatcherjavascript.h"
#include "context/qmlengineholder.h"
#include "feature/feature.h"
#include "helper.h"
#include "settings/settingsholder.h"
#include "translations/localizer.h"
#include "utilities/urlopener.h"

void TestAddonApi::env() {
  SettingsHolder settingsHolder;
  Localizer l;

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
  SettingsHolder settingsHolder;
  Localizer l;

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
  SettingsHolder settingsHolder;
  Localizer l;

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
      message, ":/addons_test/api_navigator.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
}

void TestAddonApi::settings() {
  SettingsHolder settingsHolder;
  Localizer l;

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

  settingsHolder.setAddonApiSetting(false);
  QVERIFY(!settingsHolder.addonApiSetting());

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_settings.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
  QVERIFY(settingsHolder.addonApiSetting());
}

void TestAddonApi::urlopener() {
  SettingsHolder settingsHolder;
  Localizer l;

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

  UrlOpener* uo = UrlOpener::instance();
  QVERIFY(!!uo);
  uo->registerUrlLabel("aa", []() -> QString { return "http://foo.bar"; });

  AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/api_urlopener.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());
}

void TestAddonApi::foobar() {
  AddonApi::setConstructorCallback(
      [](AddonApi* addonApi) { addonApi->insert("foobar", 42); });

  SettingsHolder settingsHolder;
  Localizer l;

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
      message, ":/addons_test/api_foobar.js");
  QVERIFY(!!a);
  QVERIFY(a->conditionApplied());

  AddonApi::setConstructorCallback(nullptr);
}

void TestAddonApi::settimedcallback() {
  SettingsHolder settingsHolder;
  Localizer l;

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
      message, ":/addons_test/api_settimedcallback.js");
  QVERIFY(!!a);

  QTimer timer;

  int timeoutPeriodMsec = 1000;

  timer.setSingleShot(true);
  timer.start(timeoutPeriodMsec);

  QSignalSpy spy(&timer, &QTimer::timeout);

  // Give the slot time to execute
  QTest::qWait(timeoutPeriodMsec + 1000);

  QObject::connect(&timer, &QTimer::timeout,
                   [&]() { QVERIFY(a->conditionApplied()); });

  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestAddonApi)
