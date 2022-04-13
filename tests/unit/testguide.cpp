/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testguide.h"
#include "../../src/models/guide.h"
#include "../../src/models/guidemodel.h"
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
  QTest::addRow("no-image")
      << QStringList{"GuideFooTitle"} << QJsonDocument(obj).toJson() << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-blocks")
      << QStringList{"GuideFooTitle"} << QJsonDocument(obj).toJson() << false;

  QJsonArray blocks;
  obj["blocks"] = blocks;
  QTest::addRow("with-blocks")
      << QStringList{"GuideFooTitle"} << QJsonDocument(obj).toJson() << true;

  blocks.append("");
  obj["blocks"] = blocks;
  QTest::addRow("with-invalid-block")
      << QStringList{"GuideFooTitle"} << QJsonDocument(obj).toJson() << false;

  QJsonObject block;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-id")
      << QStringList{"GuideFooTitle"} << QJsonDocument(obj).toJson() << false;

  block["id"] = "A";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-type")
      << QStringList{"GuideFooTitle"} << QJsonDocument(obj).toJson() << false;

  block["type"] = "wow";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-with-invalid-type")
      << QStringList{"GuideFooTitle"} << QJsonDocument(obj).toJson() << false;

  block["type"] = "title";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-title")
      << QStringList{"GuideFooTitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << true;

  block["type"] = "text";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-text")
      << QStringList{"GuideFooTitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << true;

  block["type"] = "list";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-list-without-content")
      << QStringList{"GuideFooTitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  block["content"] = "foo";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-list-with-invalid-content")
      << QStringList{"GuideFooTitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  QJsonArray content;
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-list-with-empty-content")
      << QStringList{"GuideFooTitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << true;

  content.append("foo");
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-list-with-invalid-content")
      << QStringList{"GuideFooTitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  QJsonObject subBlock;
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-list-without-id-subblock")
      << QStringList{"GuideFooTitle", "GuideFooBlockA"}
      << QJsonDocument(obj).toJson() << false;

  subBlock["id"] = "sub";
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-list-with-subblock")
      << QStringList{"GuideFooTitle", "GuideFooBlockA", "GuideFooBlockASub"}
      << QJsonDocument(obj).toJson() << true;

  obj["conditions"] = QJsonObject();
  QTest::addRow("with-block-type-list-with-subblock and conditions")
      << QStringList{"GuideFooTitle", "GuideFooBlockA", "GuideFooBlockASub"}
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

  QString guideId = guide->property("id").toString();
  QVERIFY(l18nStrings->contains(guideId));

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
           "GuideDemoTitle", "GuideDemoBlockC1", "GuideDemoBlockC2",
           "GuideDemoBlockC3L1", "GuideDemoBlockC3L2", "GuideDemoBlockC3L3"}) {
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

  QTest::addRow("empty") << QJsonObject() << true;

  {
    QJsonObject obj;
    obj["platforms"] = QJsonArray{"foo"};
    QTest::addRow("platforms") << obj << false;
  }

  QJsonObject obj;
  obj["enabledFeatures"] = QJsonArray{"appReview"};
  QTest::addRow("enabledFeatures") << obj << false;
}

void TestGuide::conditions() {
  QFETCH(QJsonObject, conditions);
  QFETCH(bool, result);
  QCOMPARE(Guide::evaluateConditions(conditions), result);
}

static TestGuide s_testGuide;
