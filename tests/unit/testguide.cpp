/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testguide.h"
#include "../../src/models/guide.h"
#include "../../src/models/guidemodel.h"
#include "../../src/settingsholder.h"
#include "helper.h"

void TestGuide::create_data() {
  QTest::addColumn<QString>("id");
  QTest::addColumn<QByteArray>("content");
  QTest::addColumn<bool>("created");

  QTest::addRow("empty") << "" << QByteArray("") << false;
  QTest::addRow("non-object") << "" << QByteArray("[]") << false;
  QTest::addRow("object-without-id") << "" << QByteArray("{}") << false;

  QJsonObject obj;
  obj["id"] = "foo";
  QTest::addRow("no-image") << "foo" << QJsonDocument(obj).toJson() << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-blocks") << "foo" << QJsonDocument(obj).toJson() << false;

  QJsonArray blocks;
  obj["blocks"] = blocks;
  QTest::addRow("with-blocks") << "foo" << QJsonDocument(obj).toJson() << true;

  blocks.append("");
  obj["blocks"] = blocks;
  QTest::addRow("with-invalid-block")
      << "foo" << QJsonDocument(obj).toJson() << false;

  QJsonObject block;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-id")
      << "foo" << QJsonDocument(obj).toJson() << false;

  block["id"] = "A";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-without-type")
      << "foo" << QJsonDocument(obj).toJson() << false;

  block["type"] = "wow";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-with-invalid-type")
      << "foo" << QJsonDocument(obj).toJson() << false;

  block["type"] = "title";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-title")
      << "foo" << QJsonDocument(obj).toJson() << true;

  block["type"] = "text";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-text")
      << "foo" << QJsonDocument(obj).toJson() << true;

  block["type"] = "olist";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-without-content")
      << "foo" << QJsonDocument(obj).toJson() << false;

  block["content"] = "foo";
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-invalid-content")
      << "foo" << QJsonDocument(obj).toJson() << false;

  QJsonArray content;
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-empty-content")
      << "foo" << QJsonDocument(obj).toJson() << true;

  content.append("foo");
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-invalid-content")
      << "foo" << QJsonDocument(obj).toJson() << false;

  QJsonObject subBlock;
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-without-id-subblock")
      << "foo" << QJsonDocument(obj).toJson() << false;

  subBlock["id"] = "sub";
  content.replace(0, subBlock);
  block["content"] = content;
  blocks.replace(0, block);
  obj["blocks"] = blocks;
  QTest::addRow("with-block-type-olist-with-subblock")
      << "foo" << QJsonDocument(obj).toJson() << true;
}

void TestGuide::create() {
  QFETCH(QString, id);
  QFETCH(QByteArray, content);
  QFETCH(bool, created);

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
  QCOMPARE(guideTitleId, QString("guide.%1.title").arg(id));
  QString guideSubTitleId = guide->property("subtitleId").toString();
  QCOMPARE(guideSubTitleId, QString("guide.%1.subtitle").arg(id));

  QCOMPARE(guide->property("image").toString(), "foo.png");

  delete guide;
}

void TestGuide::createNotExisting() {
  Guide* guide = Guide::create(nullptr, "aa");
  QVERIFY(!guide);
  delete guide;
}

void TestGuide::model() {
  SettingsHolder settingsHolder;

  GuideModel* mg = GuideModel::instance();
  QVERIFY(!!mg);

  QHash<int, QByteArray> rn = mg->roleNames();
  QCOMPARE(rn.count(), 1);
  QCOMPARE(rn[GuideModel::GuideRole], "guide");

  QCOMPARE(mg->rowCount(QModelIndex()), 0);

  QFile guideFile(":/guides/01_demo.json");
  QVERIFY(guideFile.open(QIODevice::ReadOnly | QIODevice::Text));
  QJsonDocument json = QJsonDocument::fromJson(guideFile.readAll());
  QVERIFY(json.isObject());
  mg->createFromJson("test", json.object());

  QCOMPARE(mg->rowCount(QModelIndex()), 1);
  QCOMPARE(mg->data(QModelIndex(), GuideModel::GuideRole), QVariant());

  Guide* guide =
      mg->data(mg->index(0, 0), GuideModel::GuideRole).value<Guide*>();
  QVERIFY(!!guide);
}

static TestGuide s_testGuide;
