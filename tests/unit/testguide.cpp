/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testguide.h"
#include "../../src/models/guide.h"
#include "../../src/models/guidemodel.h"
#include "../../src/settingsholder.h"
#include "../../translations/generated/l18nstrings.h"
#include "helper.h"

void TestGuide::pascalize() {
  QCOMPARE(Guide::pascalize(""), "");
  QCOMPARE(Guide::pascalize("a"), "A");
  QCOMPARE(Guide::pascalize("ab"), "Ab");
  QCOMPARE(Guide::pascalize("ab_c"), "AbC");
  QCOMPARE(Guide::pascalize("ab_cd"), "AbCd");
}

void TestGuide::create_data() {
  QTest::addColumn<QStringList>("l18n");
  QTest::addColumn<QByteArray>("content");
  QTest::addColumn<bool>("created");

  QTest::addRow("empty") << QStringList() << QByteArray("") << false;
  QTest::addRow("non-object") << QStringList() << QByteArray("[]") << false;
  QTest::addRow("object-without-id")
      << QStringList() << QByteArray("{}") << false;

  QJsonObject obj;
  obj["id"] = "foo";
  QTest::addRow("invalid-id")
      << QStringList() << QJsonDocument(obj).toJson() << false;
  QTest::addRow("no-image") << QStringList{"GuideFooTitle", "GuideFooSubtitle"}
                            << QJsonDocument(obj).toJson() << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-blocks") << QStringList{"GuideFooTitle", "GuideFooSubtitle"}
                             << QJsonDocument(obj).toJson() << false;

  QJsonArray blocks;
  obj["blocks"] = blocks;
  QTest::addRow("with-blocks")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle"}
      << QJsonDocument(obj).toJson() << true;

  blocks.append("");
  obj["blocks"] = blocks;
  QTest::addRow("with-invalid-block")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle"}
      << QJsonDocument(obj).toJson() << false;

  QJsonObject block;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-id")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle"}
      << QJsonDocument(obj).toJson() << false;

  block["id"] = "A";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-type")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle"}
      << QJsonDocument(obj).toJson() << false;

  block["type"] = "wow";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-with-invalid-type")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle"}
      << QJsonDocument(obj).toJson() << false;

  block["type"] = "title";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-title")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << true;

  block["type"] = "text";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-text")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << true;

  block["type"] = "olist";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-without-content")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  block["content"] = "foo";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-invalid-content")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  QJsonArray content;
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-empty-content")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << true;

  content.append("foo");
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-invalid-content")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  QJsonObject subBlock;
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-without-id-subblock")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  subBlock["id"] = "sub";
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-subblock")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA",
                     "GuideFooBlockASub"}
      << QJsonDocument(obj).toJson() << true;

  obj["conditions"] = QJsonObject();
  QTest::addRow("with-block-type-olist-with-subblock and conditions")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA",
                     "GuideFooBlockASub"}
      << QJsonDocument(obj).toJson() << true;

  block["type"] = "ulist";
  blocks.replace(0, block);
  obj["conditions"] = QJsonObject();
  QTest::addRow("with-block-type-ulist-with-subblock and conditions")
      << QStringList{"GuideFooTitle", "GuideFooSubtitle", "GuideFooBlockA",
                     "GuideFooBlockASub"}
      << QJsonDocument(obj).toJson() << true;
}

void TestGuide::create() {
  QFETCH(QStringList, l18n);
  QFETCH(QByteArray, content);
  QFETCH(bool, created);

  L18nStrings* l18nStrings = L18nStrings::instance();
  QVERIFY(!!l18nStrings);
  for (const QString& s : l18n) {
    l18nStrings->insert(s, "WOW!");
  }

  QTemporaryFile file;
  QVERIFY(file.open());
  QCOMPARE(file.write(content.data(), content.length()), content.length());
  file.close();

  Guide* guide = Guide::create(nullptr, file.fileName());
  QCOMPARE(!!guide, created);

  if (!guide) {
    return;
  }

  QString guideTitleId = guide->property("titleId").toString();
  QVERIFY(l18nStrings->contains(guideTitleId));
  QString guideSubTitleId = guide->property("subtitleId").toString();
  QVERIFY(l18nStrings->contains(guideSubTitleId));

  QCOMPARE(guide->property("image").toString(), "foo.png");

  delete guide;
}

void TestGuide::createNotExisting() {
  Guide* guide = Guide::create(nullptr, "aa");
  QVERIFY(!guide);
  delete guide;
}

void TestGuide::model() {
  L18nStrings* l18nStrings = L18nStrings::instance();
  QVERIFY(!!l18nStrings);
  for (const QString& s : QStringList{
           "GuideDemoTitle", "GuideDemoSubtitle", "GuideDemoBlockC1",
           "GuideDemoBlockC2", "GuideDemoBlockC3L1", "GuideDemoBlockC3L2",
           "GuideDemoBlockC3L3", "GuideDemoBlockC4L1", "GuideDemoBlockC4L2",
           "GuideDemoBlockC4L3"}) {
    l18nStrings->insert(s, "WOW!");
  }

  GuideModel* mg = GuideModel::instance();
  QVERIFY(!!mg);

  QHash<int, QByteArray> rn = mg->roleNames();
  QCOMPARE(rn.count(), 1);
  QCOMPARE(rn[GuideModel::GuideRole], "guide");

  QCOMPARE(mg->rowCount(QModelIndex()), 1);
  QCOMPARE(mg->data(QModelIndex(), GuideModel::GuideRole), QVariant());

  Guide* guide =
      mg->data(mg->index(0, 0), GuideModel::GuideRole).value<Guide*>();
  QVERIFY(!!guide);
}

void TestGuide::conditions_data() {
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

void TestGuide::conditions() {
  SettingsHolder settingsHolder;

  QFETCH(QJsonObject, conditions);
  QFETCH(bool, result);
  QFETCH(QString, settingKey);
  QFETCH(QVariant, settingValue);

  if (!settingKey.isEmpty()) {
    settingsHolder.setRawSetting(settingKey, settingValue);
  }

  QCOMPARE(Guide::evaluateConditions(conditions), result);
}

static TestGuide s_testGuide;
