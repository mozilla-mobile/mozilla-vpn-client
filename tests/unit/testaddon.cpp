/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddon.h"
#include "../../src/addons/addon.h"
#include "../../src/addons/addonguide.h"
#include "../../src/addons/addontutorial.h"
#include "../../src/settingsholder.h"
#include "../../src/qmlengineholder.h"
#include "../../src/tutorial/tutorial.h"
#include "helper.h"

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
    obj["enabledFeatures"] = QJsonArray{"appReview"};
    QTest::addRow("enabledFeatures") << obj << false << "" << QVariant();
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
}

void TestAddon::conditions() {
  SettingsHolder settingsHolder;

  QFETCH(QJsonObject, conditions);
  QFETCH(bool, result);
  QFETCH(QString, settingKey);
  QFETCH(QVariant, settingValue);

  if (!settingKey.isEmpty()) {
    settingsHolder.setRawSetting(settingKey, settingValue);
  }

  QCOMPARE(Addon::evaluateConditions(conditions), result);
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
}

void TestAddon::guide_create() {
  QFETCH(QString, id);
  QFETCH(QJsonObject, content);
  QFETCH(bool, created);

  SettingsHolder settingsHolder;

  QJsonObject obj;
  obj["guide"] = content;

  Addon* guide = AddonGuide::create(nullptr, "foo", "bar", "name", obj);
  QCOMPARE(!!guide, created);

  if (!guide) {
    return;
  }

  QString guideTitleId = guide->property("titleId").toString();
  QCOMPARE(guideTitleId, QString("guide.%1.title").arg(id));
  QString guideSubTitleId = guide->property("subtitleId").toString();
  QCOMPARE(guideSubTitleId, QString("guide.%1.subtitle").arg(id));

  QCOMPARE(guide->property("image").toString(), "foo.png");

  delete guide;
}

void TestAddon::tutorial_create_data() {
  QTest::addColumn<QString>("id");
  QTest::addColumn<QJsonObject>("content");
  QTest::addColumn<bool>("created");

  QTest::addRow("object-without-id") << "" << QJsonObject() << false;

  QJsonObject obj;
  obj["id"] = "foo";
  QTest::addRow("invalid-id") << "foo" << obj << false;
  QTest::addRow("no-image") << "foo" << obj << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-steps") << "foo" << obj << false;

  QJsonArray steps;
  obj["steps"] = steps;
  QTest::addRow("with-steps") << "foo" << obj << false;

  steps.append("");
  obj["steps"] = steps;
  QTest::addRow("with-invalid-step") << "foo" << obj << false;

  QJsonObject step;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-id") << "foo" << obj << false;

  step["id"] = "s1";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-element") << "foo" << obj << false;

  step["element"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-next") << "foo" << obj << false;

  step["next"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next") << "foo" << obj << false;

  QJsonObject nextObj;

  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-1") << "foo" << obj << false;

  nextObj["op"] = "wow";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-2") << "foo" << obj << false;

  nextObj["op"] = "signal";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-3") << "foo" << obj << false;

  nextObj["signal"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-4") << "foo" << obj << false;

  nextObj["qml_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-5") << "foo" << obj << true;

  nextObj["vpn_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-6") << "foo" << obj << false;

  nextObj.remove("qml_emitter");
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-7") << "foo" << obj << false;

  nextObj["vpn_emitter"] = "settingsHolder";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-8") << "foo" << obj << true;

  obj["conditions"] = QJsonObject();
  QTest::addRow("with-step-element and conditions") << "foo" << obj << true;
}

void TestAddon::tutorial_create() {
  QFETCH(QString, id);
  QFETCH(QJsonObject, content);
  QFETCH(bool, created);

  SettingsHolder settingsHolder;

  QJsonObject obj;
  obj["tutorial"] = content;

  Addon* tutorial = AddonTutorial::create(nullptr, "foo", "bar", "name", obj);
  QCOMPARE(!!tutorial, created);

  if (!tutorial) {
    return;
  }

  Tutorial* tm = Tutorial::instance();
  QVERIFY(!!tm);
  QVERIFY(!tm->isPlaying());

  QString tutorialTitleId = tutorial->property("titleId").toString();
  QCOMPARE(tutorialTitleId, QString("tutorial.%1.title").arg(id));

  QString tutorialSubtitleId = tutorial->property("subtitleId").toString();
  QCOMPARE(tutorialSubtitleId, QString("tutorial.%1.subtitle").arg(id));

  QString tutorialCompletionMessageId =
      tutorial->property("completionMessageId").toString();
  QCOMPARE(tutorialCompletionMessageId,
           QString("tutorial.%1.completion_message").arg(id));

  QCOMPARE(tutorial->property("image").toString(), "foo.png");

  QmlEngineHolder qml;

  QSignalSpy signalSpy(tm, &Tutorial::playingChanged);

  tm->play(tutorial);
  QCOMPARE(signalSpy.count(), 1);

  tm->stop();
  QCOMPARE(signalSpy.count(), 2);

  delete tutorial;
}

static TestAddon s_testAddon;
