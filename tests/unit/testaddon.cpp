/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddon.h"

#include <QQmlApplicationEngine>
#include <QTemporaryFile>

#include "addons/addon.h"
#include "addons/addonguide.h"
#include "addons/addonmessage.h"
#include "addons/addonproperty.h"
#include "addons/addonpropertylist.h"
#include "addons/addontutorial.h"
#include "addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h"
#include "addons/conditionwatchers/addonconditionwatchergroup.h"
#include "addons/conditionwatchers/addonconditionwatcherjavascript.h"
#include "addons/conditionwatchers/addonconditionwatcherlocales.h"
#include "addons/conditionwatchers/addonconditionwatchertimeend.h"
#include "addons/conditionwatchers/addonconditionwatchertimestart.h"
#include "addons/conditionwatchers/addonconditionwatchertriggertimesecs.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "glean/mzglean.h"
#include "helper.h"
#include "localizer.h"
#include "models/featuremodel.h"
#include "qmlengineholder.h"
#include "qtglean.h"
#include "settingsholder.h"
#include "systemtraynotificationhandler.h"
#include "tutorial/tutorial.h"

void TestAddon::init() {
  m_settingsHolder = new SettingsHolder();

  // Glean needs to be initialized for every test because this test suite
  // includes telemetry tests.
  //
  // Glean operations are queued and applied once Glean is initialized.
  // If we only initialize it in the test that actually tests telemetry all
  // of the Glean operations from previous tests will be applied and mess with
  // the state of the test that actually is testing telemetry.
  //
  // Note: on tests Glean::initialize clears Glean's storage.
  MZGlean::initialize();
}

void TestAddon::cleanup() { delete m_settingsHolder; }

void TestAddon::property() {
  AddonProperty p;
  p.initialize("foo", "bar");
  QCOMPARE(p.get(), "bar");
}

void TestAddon::property_list() {
  AddonPropertyList p;
  p.append("a", "foo");
  p.append("b", "bar");

  QStringList list{"foo", "bar"};
  QCOMPARE(p.get(), list);
  QCOMPARE(p.property("value").toStringList(), list);
}

void TestAddon::conditions_data() {
  QTest::addColumn<QJsonObject>("conditions");
  QTest::addColumn<bool>("result");
  QTest::addColumn<QString>("settingKey");
  QTest::addColumn<QVariant>("settingValue");

  QTest::addRow("empty") << QJsonObject() << true << "" << QVariant();

  {
    QJsonObject obj;
    obj["platforms"] = QJsonArray{"foo"};
    QTest::addRow("platforms") << obj << false << "" << QVariant();
  }

  {
    QJsonObject obj;
    obj["enabled_features"] = QJsonArray{"appReview"};
    QTest::addRow("enabled_features") << obj << true << "" << QVariant();
  }

  {
    QJsonObject obj;
    QJsonObject settings;
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("empty settings") << obj << false << "" << QVariant();

    settings["op"] = "eq";
    settings["setting"] = "foo";
    settings["value"] = true;
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("invalid settings") << obj << false << "" << QVariant();

    QTest::addRow("string to boolean type settings - boolean")
        << obj << true << "foo" << QVariant("wow");

    QTest::addRow("op=eq settings - boolean")
        << obj << true << "foo" << QVariant(true);

    QTest::addRow("op=eq settings - boolean 2")
        << obj << false << "foo" << QVariant(false);

    settings["op"] = "WOW";
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("invalid op settings - boolean")
        << obj << false << "foo" << QVariant(false);

    settings["op"] = "neq";
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("op=neq settings - boolean")
        << obj << true << "foo" << QVariant(false);

    settings["op"] = "eq";
    settings["value"] = 42;
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("invalid type settings - double")
        << obj << false << "foo" << QVariant("wow");

    QTest::addRow("op=eq settings - double")
        << obj << true << "foo" << QVariant(42);

    QTest::addRow("op=eq settings - double 2")
        << obj << false << "foo" << QVariant(43);

    settings["op"] = "WOW";
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("invalid op settings - double")
        << obj << false << "foo" << QVariant(43);

    settings["op"] = "neq";
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("op=neq settings - double")
        << obj << true << "foo" << QVariant(43);

    settings["op"] = "eq";
    settings["value"] = "wow";
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("invalid type settings - string")
        << obj << false << "foo" << QVariant(false);

    QTest::addRow("op=eq settings - string")
        << obj << true << "foo" << QVariant("wow");

    QTest::addRow("op=eq settings - string 2")
        << obj << false << "foo" << QVariant("wooow");

    settings["op"] = "WOW";
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("invalid op settings - string")
        << obj << false << "foo" << QVariant("woow");

    settings["op"] = "neq";
    obj["settings"] = QJsonArray{settings};
    QTest::addRow("op=neq settings - string")
        << obj << true << "foo" << QVariant("woow");
  }

  {
    QJsonObject obj;
    obj["min_client_version"] = "2.1";
    QTest::addRow("min client version ok")
        << obj << true << "" << QVariant("woow");
  }
  {
    QJsonObject obj;
    obj["min_client_version"] = "3.0";
    QTest::addRow("min client version ko")
        << obj << false << "" << QVariant("woow");
  }
  {
    QJsonObject obj;
    obj["max_client_version"] = "2.0";
    QTest::addRow("max client version ko")
        << obj << false << "" << QVariant("woow");
  }
  {
    QJsonObject obj;
    obj["max_client_version"] = "3.0";
    QTest::addRow("max client version ok")
        << obj << true << "" << QVariant("woow");
  }

  {
    QJsonObject obj;
    obj["trigger_time"] = 1;
    QTest::addRow("trigger time") << obj << true << "" << QVariant("woow");
  }

  // All of these conditions are not considered in `evaluteConditions` becaue
  // they are dynamic.
  {
    QJsonObject obj;
    obj["start_time"] = 1;
    QTest::addRow("start time (valid)")
        << obj << true << "" << QVariant("woow");
  }
  {
    QJsonObject obj;
    obj["start_time"] = QDateTime::currentSecsSinceEpoch() + 1;
    QTest::addRow("start time (expired)")
        << obj << true << "" << QVariant("woow");
  }

  {
    QJsonObject obj;
    obj["end_time"] = QDateTime::currentSecsSinceEpoch() + 1;
    QTest::addRow("end time (valid)") << obj << true << "" << QVariant("woow");
  }
  {
    QJsonObject obj;
    obj["end_time"] = QDateTime::currentSecsSinceEpoch() - 1;
    QTest::addRow("end time (expired)")
        << obj << true << "" << QVariant("woow");
  }
}

void TestAddon::conditions() {
  QFETCH(QJsonObject, conditions);
  QFETCH(bool, result);
  QFETCH(QString, settingKey);
  QFETCH(QVariant, settingValue);

  if (!settingKey.isEmpty()) {
    SettingsHolder::instance()->setRawSetting(settingKey, settingValue);
  }

  QCOMPARE(Addon::evaluateConditions(conditions), result);
}

void TestAddon::conditionWatcher_javascript() {
  MozillaVPN vpn;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);

  QVERIFY(!AddonConditionWatcherJavascript::maybeCreate(message, QString()));
  QVERIFY(!AddonConditionWatcherJavascript::maybeCreate(message, "foo"));
  QVERIFY(!AddonConditionWatcherJavascript::maybeCreate(
      message, ":/addons_test/condition1.js"));

  {
    AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
        message, ":/addons_test/condition2.js");
    QVERIFY(!!a);
    QVERIFY(!a->conditionApplied());
  }

  {
    AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
        message, ":/addons_test/condition3.js");
    QVERIFY(!!a);
    QVERIFY(a->conditionApplied());
  }

  {
    AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
        message, ":/addons_test/condition4.js");
    QVERIFY(!!a);
    QVERIFY(!a->conditionApplied());

    QEventLoop loop;
    bool currentStatus = false;
    connect(a, &AddonConditionWatcher::conditionChanged, a, [&](bool status) {
      currentStatus = status;
      loop.exit();
    });
    loop.exec();
    QVERIFY(currentStatus);
    loop.exec();
    QVERIFY(!currentStatus);
  }

  {
    AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
        message, ":/addons_test/condition5.js");
    QVERIFY(!!a);
    QVERIFY(!a->conditionApplied());

    SettingsHolder::instance()->setStartAtBoot(true);
    QVERIFY(a->conditionApplied());
  }
}

void TestAddon::conditionWatcher_locale() {
  Localizer l;

  QObject parent;

  // No locales -> no watcher.
  QVERIFY(!AddonConditionWatcherLocales::maybeCreate(&parent, QStringList()));

  AddonConditionWatcher* acw =
      AddonConditionWatcherLocales::maybeCreate(&parent, QStringList{"it"});
  QVERIFY(!!acw);

  QSignalSpy signalSpy(acw, &AddonConditionWatcher::conditionChanged);
  QCOMPARE(signalSpy.count(), 0);
  SettingsHolder::instance()->setLanguageCode("en");
  QCOMPARE(signalSpy.count(), 0);
  SettingsHolder::instance()->setLanguageCode("it_RU");
  QCOMPARE(signalSpy.count(), 1);
  SettingsHolder::instance()->setLanguageCode("it");
  QCOMPARE(signalSpy.count(), 1);
  SettingsHolder::instance()->setLanguageCode("es");
  QCOMPARE(signalSpy.count(), 2);

  SettingsHolder::instance()->setLanguageCode("en");
  QVERIFY(!acw->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("it");
  QVERIFY(acw->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("ru");
  QVERIFY(!acw->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("it-IT");
  QVERIFY(acw->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("en");
  QVERIFY(!acw->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("it_RU");
  QVERIFY(acw->conditionApplied());
}

void TestAddon::conditionWatcher_featuresEnabled() {
  QObject parent;

  // Empty feature list
  QVERIFY(!AddonConditionWatcherFeaturesEnabled::maybeCreate(&parent,
                                                             QStringList()));

  // Invalid feature list
  QVERIFY(!AddonConditionWatcherFeaturesEnabled::maybeCreate(
      &parent, QStringList{"invalid"}));

  QVERIFY(!Feature::getOrNull("testFeatureAddon"));
  Feature feature(
      "testFeatureAddon", "Feature Addon",
      []() -> bool { return true; },  // Can be flipped on
      []() -> bool { return true; },  // Can be flipped off
      QStringList(),                  // feature dependencies
      []() -> bool { return false; });
  QVERIFY(!!Feature::get("testFeatureAddon"));
  QVERIFY(!Feature::get("testFeatureAddon")->isSupported());

  // A condition not enabled by default
  AddonConditionWatcher* acw =
      AddonConditionWatcherFeaturesEnabled::maybeCreate(
          &parent, QStringList{"testFeatureAddon"});
  QVERIFY(!!acw);
  QVERIFY(!acw->conditionApplied());

  QSignalSpy signalSpy(acw, &AddonConditionWatcher::conditionChanged);
  QCOMPARE(signalSpy.count(), 0);

  FeatureModel* fm = FeatureModel::instance();

  fm->toggle("testFeatureAddon");
  QVERIFY(Feature::get("testFeatureAddon")->isSupported());
  QCOMPARE(signalSpy.count(), 1);
  QVERIFY(acw->conditionApplied());

  // A condition enabled by default
  {
    AddonConditionWatcher* acw2 =
        AddonConditionWatcherFeaturesEnabled::maybeCreate(
            &parent, QStringList{"testFeatureAddon"});
    QVERIFY(!!acw2);
    QVERIFY(acw2->conditionApplied());
  }

  fm->toggle("testFeatureAddon");
  QVERIFY(!Feature::get("testFeatureAddon")->isSupported());
  QCOMPARE(signalSpy.count(), 2);
  QVERIFY(!acw->conditionApplied());
}

void TestAddon::conditionWatcher_group() {
  SettingsHolder::instance()->setInstallationTime(QDateTime::currentDateTime());

  QObject parent;
  AddonConditionWatcher* acw1 =
      AddonConditionWatcherTriggerTimeSecs::maybeCreate(&parent, 1);
  QVERIFY(!!acw1);
  QVERIFY(!acw1->conditionApplied());

  AddonConditionWatcher* acw2 =
      AddonConditionWatcherTriggerTimeSecs::maybeCreate(&parent, 2);
  QVERIFY(!!acw2);
  QVERIFY(!acw2->conditionApplied());

  AddonConditionWatcher* acwGroup = new AddonConditionWatcherGroup(
      &parent, QList<AddonConditionWatcher*>{acw1, acw2});
  QVERIFY(!acwGroup->conditionApplied());

  QEventLoop loop;
  bool currentStatus = false;
  connect(acw1, &AddonConditionWatcher::conditionChanged, acw1,
          [&](bool status) {
            currentStatus = status;
            loop.exit();
          });
  loop.exec();

  QVERIFY(currentStatus);
  QVERIFY(acw1->conditionApplied());
  QVERIFY(!acw2->conditionApplied());
  QVERIFY(!acwGroup->conditionApplied());

  currentStatus = false;
  connect(acw2, &AddonConditionWatcher::conditionChanged, acw2,
          [&](bool status) {
            currentStatus = status;
            loop.exit();
          });
  loop.exec();

  QVERIFY(currentStatus);
  QVERIFY(acw1->conditionApplied());
  QVERIFY(acw2->conditionApplied());
  QVERIFY(acwGroup->conditionApplied());
}

void TestAddon::conditionWatcher_triggerTime() {
  SettingsHolder::instance()->setInstallationTime(QDateTime::currentDateTime());

  QObject parent;
  AddonConditionWatcher* acw =
      AddonConditionWatcherTriggerTimeSecs::maybeCreate(&parent, 1);
  QVERIFY(!!acw);

  QVERIFY(!acw->conditionApplied());

  QEventLoop loop;
  bool currentStatus = false;
  connect(acw, &AddonConditionWatcher::conditionChanged, acw, [&](bool status) {
    currentStatus = status;
    loop.exit();
  });
  loop.exec();

  QVERIFY(currentStatus);
  QVERIFY(acw->conditionApplied());
}

void TestAddon::conditionWatcher_startTime() {
  QObject parent;
  AddonConditionWatcher* acw = new AddonConditionWatcherTimeStart(&parent, 0);
  QVERIFY(acw->conditionApplied());

  acw = new AddonConditionWatcherTimeStart(
      &parent, QDateTime::currentSecsSinceEpoch() + 1);
  QVERIFY(!acw->conditionApplied());

  QEventLoop loop;
  bool currentStatus = false;
  connect(acw, &AddonConditionWatcher::conditionChanged, acw, [&](bool status) {
    currentStatus = status;
    loop.exit();
  });
  loop.exec();

  QVERIFY(currentStatus);
  QVERIFY(acw->conditionApplied());
}

void TestAddon::conditionWatcher_endTime() {
  QObject parent;
  AddonConditionWatcher* acw = new AddonConditionWatcherTimeEnd(&parent, 0);
  QVERIFY(!acw->conditionApplied());

  acw = new AddonConditionWatcherTimeEnd(
      &parent, QDateTime::currentSecsSinceEpoch() + 1);
  QVERIFY(acw->conditionApplied());

  QEventLoop loop;
  bool currentStatus = false;
  connect(acw, &AddonConditionWatcher::conditionChanged, acw, [&](bool status) {
    currentStatus = status;
    loop.exit();
  });
  loop.exec();

  QVERIFY(!currentStatus);
  QVERIFY(!acw->conditionApplied());
}

void TestAddon::guide_create_data() {
  QTest::addColumn<QString>("id");
  QTest::addColumn<QJsonObject>("content");
  QTest::addColumn<bool>("created");

  QTest::addRow("object-without-id") << "" << QJsonObject() << false;

  QJsonObject obj;
  obj["id"] = "foo";
  QTest::addRow("no-image") << "foo" << obj << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-blocks") << "foo" << obj << false;

  QJsonArray blocks;
  obj["blocks"] = blocks;
  QTest::addRow("with-blocks") << "foo" << obj << true;

  blocks.append("");
  obj["blocks"] = blocks;
  QTest::addRow("with-invalid-block") << "foo" << obj << false;

  QJsonObject block;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-id") << "foo" << obj << false;

  block["id"] = "A";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-type") << "foo" << obj << false;

  block["type"] = "wow";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-with-invalid-type") << "foo" << obj << false;

  block["type"] = "title";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-title") << "foo" << obj << true;

  block["type"] = "text";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-text") << "foo" << obj << true;

  block["type"] = "olist";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-without-content")
      << "foo" << obj << false;

  block["content"] = "foo";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-invalid-content")
      << "foo" << obj << false;

  QJsonArray content;
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-empty-content")
      << "foo" << obj << true;

  content.append("foo");
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-invalid-content")
      << "foo" << obj << false;

  QJsonObject subBlock;
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-without-id-subblock")
      << "foo" << obj << false;

  subBlock["id"] = "sub";
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-subblock") << "foo" << obj << true;

  obj["advanced"] = true;
  QTest::addRow("advanced") << "foo" << obj << true;

  obj["advanced"] = false;
  QTest::addRow("not-advanced") << "foo" << obj << true;
}

void TestAddon::guide_create() {
  QFETCH(QString, id);
  QFETCH(QJsonObject, content);
  QFETCH(bool, created);

  QJsonObject obj;
  obj["guide"] = content;

  QObject parent;
  Addon* guide = AddonGuide::create(&parent, id, "bar", "name", obj);
  QCOMPARE(!!guide, created);

  if (!guide) {
    return;
  }

  QCOMPARE(guide->property("title").type(), QMetaType::QString);
  QCOMPARE(guide->property("subtitle").type(), QMetaType::QString);

  QCOMPARE(guide->property("image").toString(), "foo.png");
  QCOMPARE(guide->property("advanced").toBool(), content["advanced"].toBool());
}

void TestAddon::tutorial_create_data() {
  QTest::addColumn<QString>("id");
  QTest::addColumn<QJsonObject>("content");
  QTest::addColumn<bool>("created");
  QTest::addColumn<bool>("highlighted");
  QTest::addColumn<bool>("transaction");

  QTest::addRow("object-without-id")
      << "" << QJsonObject() << false << false << false;

  QJsonObject obj;
  obj["id"] = "foo";
  QTest::addRow("invalid-id") << "foo" << obj << false << false << false;
  QTest::addRow("no-image") << "foo" << obj << false << false << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-steps") << "foo" << obj << false << false << false;

  QJsonArray steps;
  obj["steps"] = steps;
  QTest::addRow("with-steps") << "foo" << obj << false << false << false;

  steps.append("");
  obj["steps"] = steps;
  QTest::addRow("with-invalid-step") << "foo" << obj << false << false << false;

  QJsonObject step;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-id")
      << "foo" << obj << false << false << false;

  step["id"] = "s1";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-query")
      << "foo" << obj << false << false << false;

  step["query"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-next")
      << "foo" << obj << false << false << false;

  step["next"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next")
      << "foo" << obj << false << false << false;

  QJsonObject nextObj;

  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-1")
      << "foo" << obj << false << false << false;

  nextObj["op"] = "wow";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-2")
      << "foo" << obj << false << false << false;

  nextObj["op"] = "signal";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-3")
      << "foo" << obj << false << false << false;

  nextObj["signal"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-4")
      << "foo" << obj << false << false << false;

  nextObj["query_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-5")
      << "foo" << obj << true << false << false;

  nextObj["vpn_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-6")
      << "foo" << obj << false << false << false;

  nextObj.remove("query_emitter");
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-7")
      << "foo" << obj << false << false << false;

  nextObj["vpn_emitter"] = "settingsHolder";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-8")
      << "foo" << obj << true << false << false;

  obj["conditions"] = QJsonObject();
  QTest::addRow("with-step-query and conditions")
      << "foo" << obj << true << false << false;

  obj["highlighted"] = true;
  QTest::addRow("highlighted") << "foo" << obj << true << true << false;

  obj["settings_rollback_needed"] = true;
  QTest::addRow("advanced-and-highlighted")
      << "foo" << obj << true << true << true;
}

void TestAddon::tutorial_create() {
  QFETCH(QString, id);
  QFETCH(QJsonObject, content);
  QFETCH(bool, created);
  QFETCH(bool, highlighted);
  QFETCH(bool, transaction);

  QJsonObject obj;
  obj["tutorial"] = content;

  QObject parent;
  Addon* tutorial = AddonTutorial::create(&parent, id, "bar", "name", obj);
  QCOMPARE(!!tutorial, created);

  if (!tutorial) {
    return;
  }

  Tutorial* tm = Tutorial::instance();
  QVERIFY(!!tm);
  QVERIFY(!tm->isPlaying());

  QCOMPARE(tutorial->property("title").type(), QMetaType::QString);
  QCOMPARE(tutorial->property("subtitle").type(), QMetaType::QString);
  QCOMPARE(tutorial->property("completionMessage").type(), QMetaType::QString);
  QCOMPARE(tutorial->property("image").toString(), "foo.png");
  QCOMPARE(tutorial->property("highlighted").toBool(), highlighted);
  QCOMPARE(tutorial->property("settingsRollbackNeeded").toBool(), transaction);

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QSignalSpy signalSpy(tm, &Tutorial::playingChanged);

  QVERIFY(!SettingsHolder::instance()->inTransaction());

  tm->play(tutorial);
  QCOMPARE(signalSpy.count(), 1);

  QCOMPARE(SettingsHolder::instance()->inTransaction(), transaction);

  tm->stop();
  QCOMPARE(signalSpy.count(), 2);

  QVERIFY(!SettingsHolder::instance()->inTransaction());
}

void TestAddon::message_create_data() {
  QTest::addColumn<QString>("id");
  QTest::addColumn<QJsonObject>("content");
  QTest::addColumn<bool>("created");

  QTest::addRow("object-without-id") << "" << QJsonObject() << false;

  QJsonObject obj;
  obj["id"] = "foo";
  QTest::addRow("no blocks") << "foo" << obj << false;

  obj["blocks"] = QJsonArray();
  QTest::addRow("good but empty") << "foo" << obj << true;
}

void TestAddon::message_create() {
  QFETCH(QString, id);
  QFETCH(QJsonObject, content);
  QFETCH(bool, created);

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QCOMPARE(!!message, created);

  if (!message) {
    return;
  }

  QCOMPARE(message->property("title").type(), QMetaType::QString);
}

void TestAddon::message_load_state_data() {
  QTest::addColumn<AddonMessage::MessageState>("state");
  QTest::addColumn<QString>("setting");

  QTest::addRow("empty-setting") << AddonMessage::MessageState::Received << "";
  QTest::addRow("wrong-setting")
      << AddonMessage::MessageState::Received << "WRONG!";

  QTest::addRow("received")
      << AddonMessage::MessageState::Received << "Received";
  QTest::addRow("notified")
      << AddonMessage::MessageState::Notified << "Notified";
  QTest::addRow("read") << AddonMessage::MessageState::Read << "Read";
  QTest::addRow("dismissed")
      << AddonMessage::MessageState::Dismissed << "Dismissed";
}

void TestAddon::message_load_state() {
  QFETCH(AddonMessage::MessageState, state);
  QFETCH(QString, setting);

  SettingsHolder::instance()->setAddonSetting(
      AddonMessage::MessageStateQuery("foo"), setting);
  QCOMPARE(AddonMessage::loadMessageState("foo"), state);
}

void TestAddon::message_notification_data() {
  SettingsHolder settingsHolder;
  Localizer l;

  QObject parent;
  SystemTrayNotificationHandler nh(&parent);

  QTest::addColumn<QString>("title");
  QTest::addColumn<QString>("message");
  QTest::addColumn<QString>("actual_title");
  QTest::addColumn<QString>("actual_message");

  TestHelper::resetLastSystemNotification();
  // Message is created for the first time,
  // but user is not logged in, no  message sent
  QTest::addRow("not-logged-in")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  // Message is created for the first time,
  // user is not logged in and message is disabled, no  message sent
  AddonMessage* disabledPreLoginMessage = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message2.json"));
  QTest::addRow("not-logged-in-disabled")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  // Mock a user login.
  TestHelper::resetLastSystemNotification();
  TestHelper::userState = MozillaVPN::UserAuthenticated;
  emit MozillaVPN::instance()->userStateChanged();
  // A login should not trigger any messages either.
  QTest::addRow("login") << QString() << QString()
                         << TestHelper::lastSystemNotification.title
                         << TestHelper::lastSystemNotification.message;

  // Message received pre login is enabled post login, message sent
  TestHelper::resetLastSystemNotification();
  // Message is later enabled
  disabledPreLoginMessage->enable();
  QTest::addRow("enable-post-login")
      << QString("Test Message 2 - Title")
      << QString("Test Message 2 - Subtitle")
      << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is created for the first time, notification should be sent
  AddonMessage* message = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message3.json"));
  QTest::addRow("do-show") << QString("Test Message 3 - Title")
                           << QString("Test Message 3 - Subtitle")
                           << TestHelper::lastSystemNotification.title
                           << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is created for the second time, notification should not be sent
  Addon::create(&parent, ":/addons_test/message3.json");
  QTest::addRow("do-not-show")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is marked as read and we re-attempt to send a notification
  message->markAsRead();
  message->maybePushNotification();
  QTest::addRow("message-is-read")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Another message is created for the first time
  AddonMessage* anotherMessage = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message4.json"));
  QTest::addRow("do-show-2") << QString("Test Message 4 - Title")
                             << QString("Test Message 4 - Subtitle")
                             << TestHelper::lastSystemNotification.title
                             << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is dismissed and we re-attempt to send a notification
  anotherMessage->dismiss();
  anotherMessage->maybePushNotification();
  QTest::addRow("message-dismissed")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is created but due to it"s conditions it"s not enabled
  AddonMessage* disabledMessage = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message5.json"));
  QTest::addRow("message-loaded-disabled")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is later enabled
  disabledMessage->enable();
  QTest::addRow("message-enabled")
      << QString("Test Message 5 - Title")
      << QString("Test Message 5 - Subtitle")
      << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;
}

void TestAddon::message_notification() {
  QFETCH(QString, title);
  QFETCH(QString, message);
  QFETCH(QString, actual_title);
  QFETCH(QString, actual_message);

  QCOMPARE(actual_title, title);
  QCOMPARE(actual_message, message);
}

void TestAddon::message_date_data() {
  QTest::addColumn<QString>("languageCode");
  QTest::addColumn<QDateTime>("now");
  QTest::addColumn<QDateTime>("date");
  QTest::addColumn<QString>("result");
  QTest::addColumn<qint64>("timer");

  QTest::addRow("en - future")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(11, 0), QTimeZone(0)) << "10:00 AM"
      << (qint64)(14 * 3600);
  QTest::addRow("it - future")
      << "it" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(11, 0), QTimeZone(0)) << "10:00"
      << (qint64)(14 * 3600);

  QTest::addRow("en - same")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0)) << "10:00 AM"
      << (qint64)(14 * 3600);
  QTest::addRow("it - same")
      << "it" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0)) << "10:00"
      << (qint64)(14 * 3600);

  QTest::addRow("en - one hour ago")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(9, 0), QTimeZone(0)) << "9:00 AM"
      << (qint64)(15 * 3600);
  QTest::addRow("it - one hour ago")
      << "it" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(9, 0), QTimeZone(0)) << "09:00"
      << (qint64)(15 * 3600);

  QTest::addRow("en - midnight")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(0, 0), QTimeZone(0)) << "12:00 AM"
      << (qint64)(24 * 3600);
  QTest::addRow("it - midnight")
      << "it" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(0, 0), QTimeZone(0)) << "00:00"
      << (qint64)(24 * 3600);

  QTest::addRow("en - yesterday but less than 24 hours")
      << "en" << QDateTime(QDate(2000, 1, 2), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(21, 0), QTimeZone(0)) << "Yesterday"
      << (qint64)(3 * 3600);

  QTest::addRow("en - yesterday more than 24 hours")
      << "en" << QDateTime(QDate(2000, 1, 2), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(9, 0), QTimeZone(0)) << "Yesterday"
      << (qint64)-1;

  QTest::addRow("en - 2 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 8), QTime(10, 0), QTimeZone(0)) << "Saturday"
      << (qint64)-1;

  QTest::addRow("en - 3 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 7), QTime(10, 0), QTimeZone(0)) << "Friday"
      << (qint64)-1;

  QTest::addRow("en - 4 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 6), QTime(10, 0), QTimeZone(0)) << "Thursday"
      << (qint64)-1;

  QTest::addRow("en - 5 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 5), QTime(10, 0), QTimeZone(0)) << "Wednesday"
      << (qint64)-1;

  QTest::addRow("en - 6 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 4), QTime(10, 0), QTimeZone(0)) << "Tuesday"
      << (qint64)-1;

  QTest::addRow("en - 7 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 3), QTime(10, 0), QTimeZone(0)) << "1/3/00"
      << (qint64)-1;
}

void TestAddon::message_date() {
  Localizer localizer;

  QFETCH(QString, languageCode);
  SettingsHolder::instance()->setLanguageCode(languageCode);

  QFETCH(QDateTime, now);
  QVERIFY(now.isValid());

  QFETCH(QDateTime, date);
  QVERIFY(date.isValid());

  QFETCH(QString, result);
  QCOMPARE(Localizer::instance()->formatDate(date), result);

  QFETCH(qint64, timer);
  QCOMPARE(AddonMessage::planDateRetranslationInternal(now, date), timer);
}

void TestAddon::message_dismiss() {
  Localizer l;

  QJsonObject messageObj;
  messageObj["id"] = "foo";
  messageObj["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = messageObj;
  obj["type"] = "message";
  obj["api_version"] = "0.1";
  obj["id"] = "bar";
  obj["name"] = "bar";

  QTemporaryFile file;
  QVERIFY(file.open());
  file.write(QJsonDocument(obj).toJson());
  file.close();

  QObject parent;
  Addon* message = Addon::create(&parent, file.fileName());
  QVERIFY(!!message);
  QVERIFY(message->enabled());

  QString addonSetting;
  connect(SettingsHolder::instance(), &SettingsHolder::addonSettingsChanged,
          [&]() {
            addonSetting = SettingsHolder::instance()->getAddonSetting(
                SettingsHolder::AddonSettingQuery(
                    "bar", ADDON_MESSAGE_SETTINGS_GROUP,
                    ADDON_MESSAGE_SETTINGS_STATE_KEY, "?!?"));
          });

  QCOMPARE(addonSetting, "");
  QVERIFY(!static_cast<AddonMessage*>(message)->isRead());

  static_cast<AddonMessage*>(message)->markAsRead();
  QVERIFY(static_cast<AddonMessage*>(message)->isRead());
  QCOMPARE(addonSetting, "Read");

  // After dismissing the message, it becomes inactive.
  static_cast<AddonMessage*>(message)->dismiss();
  QVERIFY(!message->enabled());
  QCOMPARE(addonSetting, "Dismissed");

  // No new messages are loaded for the same ID:
  Addon* message2 = AddonMessage::create(&parent, "foo", "bar", "name", obj);
  QVERIFY(!message2);
}

void TestAddon::telemetry_state_change() {
  Localizer localizer;

  QJsonObject content;
  content["id"] = "foo";
  content["image"] = "foo.png";
  content["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["guide"] = content;
  obj["type"] = "guide";
  obj["api_version"] = "0.1";
  obj["id"] = "id";
  obj["name"] = "name";

  QTemporaryFile file;
  QVERIFY(file.open());
  file.write(QJsonDocument(obj).toJson());
  file.close();

  QObject parent;
  // The type of the addon, for the purposes of this test, is irrelevant.
  Addon* guide = Addon::create(&parent, file.fileName());

  // Check no errors were recorded for this metric.
  // Event metrics can only record InvalidValue or InvalidOverflow errors
  // https://mozilla.github.io/glean/book/reference/metrics/event.html#recorded-errors

  QCOMPARE(mozilla::glean::sample::addon_state_changed.testGetNumRecordedErrors(
               ErrorType::InvalidValue),
           0);
  QCOMPARE(mozilla::glean::sample::addon_state_changed.testGetNumRecordedErrors(
               ErrorType::InvalidOverflow),
           0);

  // Upon creating the addon we expect two events to be recorded:
  // Installed and then Enabled/Disabled. In this case the addon is enabled.
  // So we expect an Enabled state event to be recorded.

  auto initialValues =
      mozilla::glean::sample::addon_state_changed.testGetValue();

  QCOMPARE(initialValues.length(), 2);

  auto installedEventExtras = initialValues[0]["extra"].toObject();
  QCOMPARE(installedEventExtras["addon_id"].toString(), "id");
  QCOMPARE(installedEventExtras["state"].toString(), "Installed");

  auto enabledEventExtras = initialValues[1]["extra"].toObject();
  QCOMPARE(enabledEventExtras["addon_id"].toString(), "id");
  QCOMPARE(enabledEventExtras["state"].toString(), "Enabled");

  guide->disable();

  // After disabling we expect a disabled state event to be recorded.

  QCOMPARE(mozilla::glean::sample::addon_state_changed.testGetNumRecordedErrors(
               ErrorType::InvalidValue),
           0);
  QCOMPARE(mozilla::glean::sample::addon_state_changed.testGetNumRecordedErrors(
               ErrorType::InvalidOverflow),
           0);

  auto postDisableValues =
      mozilla::glean::sample::addon_state_changed.testGetValue();

  QCOMPARE(postDisableValues.length(), 3);

  auto disabledEventExtras = postDisableValues[2]["extra"].toObject();
  QCOMPARE(disabledEventExtras["addon_id"].toString(), "id");
  QCOMPARE(disabledEventExtras["state"].toString(), "Disabled");

  // Lastly, let's check nothing is recorded if an addon creation fails.

  Addon::create(&parent, "does_not_exist.json");
  auto postFailedCreationValues =
      mozilla::glean::sample::addon_state_changed.testGetValue();

  QCOMPARE(postDisableValues.length(), 3);
}

static TestAddon s_testAddon;
