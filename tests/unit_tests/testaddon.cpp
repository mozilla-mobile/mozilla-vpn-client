/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddon.h"

#include <QQmlApplicationEngine>
#include <QTemporaryFile>

#include "addons/addon.h"
#include "addons/addonmessage.h"
#include "addons/addonproperty.h"
#include "addons/addonpropertylist.h"
#include "addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h"
#include "addons/conditionwatchers/addonconditionwatchergroup.h"
#include "addons/conditionwatchers/addonconditionwatcherjavascript.h"
#include "addons/conditionwatchers/addonconditionwatcherlocales.h"
#include "addons/conditionwatchers/addonconditionwatchertimeend.h"
#include "addons/conditionwatchers/addonconditionwatchertimestart.h"
#include "addons/conditionwatchers/addonconditionwatchertriggertimesecs.h"
#include "feature/feature.h"
#include "feature/featuremodel.h"
#include "glean/generated/metrics.h"
#include "glean/mzglean.h"
#include "helper.h"
#include "localizer.h"
#include "qmlengineholder.h"
#include "qtglean.h"
#include "settings/settingsmanager.h"
#include "settingsholder.h"

void TestAddon::init() {
  m_settingsHolder = new SettingsHolder();

  // Glean needs to be initialized for every test because this test suite
  // includes telemetry tests.
  //
  // Glean operations are queued and applied once Glean is initialized.
  // If we only initialize it in the test that actually tests telemetry all
  // of the Glean operations from previous tests will be applied and mess with
  // the status of the test that actually is testing telemetry.
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
    auto setting = SettingsManager::instance()->getSetting(settingKey);
    if (!setting) {
      SettingsManager::instance()->createOrGetSetting(
          settingKey, []() { return QVariant(); }, true, false);
      setting = SettingsManager::instance()->getSetting(settingKey);
    }

    setting->set(settingValue);
  }

  QCOMPARE(Addon::evaluateConditions(conditions), result);
}

void TestAddon::conditionWatcher_javascript() {
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
    SettingsHolder::instance()->setAddonApiSetting(false);

    AddonConditionWatcher* a = AddonConditionWatcherJavascript::maybeCreate(
        message, ":/addons_test/condition5.js");
    QVERIFY(!!a);
    QVERIFY(!a->conditionApplied());

    SettingsHolder::instance()->setAddonApiSetting(true);
    QVERIFY(a->conditionApplied());
  }
}

void TestAddon::conditionWatcher_locale() {
  Localizer l;

  QObject parent;

  // No locales -> no watcher.
  QVERIFY(!AddonConditionWatcherLocales::maybeCreate(&parent, QStringList()));

  AddonConditionWatcher* acw = AddonConditionWatcherLocales::maybeCreate(
      &parent, QStringList{"it", "fo_BAR"});
  QVERIFY(!!acw);

  AddonConditionWatcher* acw2 = AddonConditionWatcherLocales::maybeCreate(
      &parent, QStringList{"it", "fo_BAR"},
      AddonConditionWatcherLocales::DoNotCheckMajorLanguageCode);
  QVERIFY(!!acw2);

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
  QVERIFY(!acw2->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("it");
  QVERIFY(acw->conditionApplied());
  QVERIFY(acw2->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("ru");
  QVERIFY(!acw->conditionApplied());
  QVERIFY(!acw2->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("it-IT");
  QVERIFY(acw->conditionApplied());
  QVERIFY(!acw2->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("en");
  QVERIFY(!acw->conditionApplied());
  QVERIFY(!acw2->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("it_RU");
  QVERIFY(acw->conditionApplied());
  QVERIFY(!acw2->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("fo");
  QVERIFY(!acw->conditionApplied());
  QVERIFY(!acw2->conditionApplied());

  SettingsHolder::instance()->setLanguageCode("fo_BAR");
  QVERIFY(acw->conditionApplied());
  QVERIFY(acw2->conditionApplied());
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

void TestAddon::message_load_status_data() {
  QTest::addColumn<AddonMessage::MessageStatus>("status");
  QTest::addColumn<QString>("setting");

  QTest::addRow("empty-setting") << AddonMessage::MessageStatus::Received << "";
  QTest::addRow("wrong-setting")
      << AddonMessage::MessageStatus::Received << "WRONG!";

  QTest::addRow("received")
      << AddonMessage::MessageStatus::Received << "Received";
  QTest::addRow("notified")
      << AddonMessage::MessageStatus::Notified << "Notified";
  QTest::addRow("read") << AddonMessage::MessageStatus::Read << "Read";
  QTest::addRow("dismissed")
      << AddonMessage::MessageStatus::Dismissed << "Dismissed";
}

void TestAddon::message_load_status() {
  Localizer l;

  QFETCH(AddonMessage::MessageStatus, status);
  QFETCH(QString, setting);

  QJsonObject messageObj;
  messageObj["id"] = "foo";
  messageObj["blocks"] = QJsonArray();

  QJsonObject obj;
  obj["message"] = messageObj;
  obj["type"] = "message";
  obj["api_version"] = "0.1";
  obj["id"] = "foo";
  obj["name"] = "foo";

  QTemporaryFile file;
  QVERIFY(file.open());
  file.write(QJsonDocument(obj).toJson());
  file.close();

  QObject parent;
  Addon* message = Addon::create(&parent, file.fileName());

  static_cast<AddonMessage*>(message)->m_messageSettingGroup->set(
      ADDON_MESSAGE_SETTINGS_STATUS_KEY, (setting));
  QCOMPARE(static_cast<AddonMessage*>(message)->loadMessageStatus("foo"),
           status);
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
  connect(static_cast<AddonMessage*>(message)->m_messageSettingGroup,
          &SettingGroup::changed, &parent, [&]() {
            addonSetting = static_cast<AddonMessage*>(message)
                               ->m_messageSettingGroup
                               ->get(ADDON_MESSAGE_SETTINGS_STATUS_KEY)
                               .toString();
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

static TestAddon s_testAddon;
