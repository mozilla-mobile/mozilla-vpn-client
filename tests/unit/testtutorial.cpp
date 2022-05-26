/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtutorial.h"
#include "../../src/models/tutorial.h"
#include "../../src/models/tutorialmodel.h"
#include "../../src/qmlengineholder.h"
#include "../../src/settingsholder.h"

void TestTutorial::model() {
  SettingsHolder settingsHolder;

  TutorialModel* mg = TutorialModel::instance();
  QVERIFY(!!mg);

  QHash<int, QByteArray> rn = mg->roleNames();
  QCOMPARE(rn.count(), 1);
  QCOMPARE(rn[TutorialModel::TutorialRole], "tutorial");

  QCOMPARE(mg->rowCount(QModelIndex()), 0);

  QFile tutorialFile(":/tutorials/01_demo.json");
  QVERIFY(tutorialFile.open(QIODevice::ReadOnly | QIODevice::Text));
  QJsonDocument json = QJsonDocument::fromJson(tutorialFile.readAll());
  QVERIFY(json.isObject());
  mg->createFromJson(json.object());

  QCOMPARE(mg->rowCount(QModelIndex()), 1);
  QCOMPARE(mg->data(QModelIndex(), TutorialModel::TutorialRole), QVariant());

  Tutorial* tutorial =
      mg->data(mg->index(0, 0), TutorialModel::TutorialRole).value<Tutorial*>();
  QVERIFY(!!tutorial);
}

void TestTutorial::create_data() {
  QTest::addColumn<QString>("id");
  QTest::addColumn<QByteArray>("content");
  QTest::addColumn<bool>("created");

  QTest::addRow("empty") << "" << QByteArray("") << false;
  QTest::addRow("non-object") << "" << QByteArray("[]") << false;
  QTest::addRow("object-without-id") << "" << QByteArray("{}") << false;

  QJsonObject obj;
  obj["id"] = "foo";
  QTest::addRow("invalid-id") << "foo" << QJsonDocument(obj).toJson() << false;
  QTest::addRow("no-image") << "foo" << QJsonDocument(obj).toJson() << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-steps") << "foo" << QJsonDocument(obj).toJson() << false;

  QJsonArray steps;
  obj["steps"] = steps;
  QTest::addRow("with-steps") << "foo" << QJsonDocument(obj).toJson() << false;

  steps.append("");
  obj["steps"] = steps;
  QTest::addRow("with-invalid-step")
      << "foo" << QJsonDocument(obj).toJson() << false;

  QJsonObject step;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-id")
      << "foo" << QJsonDocument(obj).toJson() << false;

  step["id"] = "s1";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-element")
      << "foo" << QJsonDocument(obj).toJson() << false;

  step["element"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-next")
      << "foo" << QJsonDocument(obj).toJson() << false;

  step["next"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next")
      << "foo" << QJsonDocument(obj).toJson() << false;

  QJsonObject nextObj;

  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-1")
      << "foo" << QJsonDocument(obj).toJson() << false;

  nextObj["op"] = "wow";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-2")
      << "foo" << QJsonDocument(obj).toJson() << false;

  nextObj["op"] = "signal";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-3")
      << "foo" << QJsonDocument(obj).toJson() << false;

  nextObj["signal"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-4")
      << "foo" << QJsonDocument(obj).toJson() << false;

  nextObj["qml_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-5")
      << "foo" << QJsonDocument(obj).toJson() << true;

  nextObj["vpn_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-6")
      << "foo" << QJsonDocument(obj).toJson() << false;

  nextObj.remove("qml_emitter");
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-7")
      << "foo" << QJsonDocument(obj).toJson() << false;

  nextObj["vpn_emitter"] = "settingsHolder";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-8")
      << "foo" << QJsonDocument(obj).toJson() << true;

  obj["conditions"] = QJsonObject();
  QTest::addRow("with-step-element and conditions")
      << "foo" << QJsonDocument(obj).toJson() << true;
}

void TestTutorial::create() {
  QFETCH(QString, id);
  QFETCH(QByteArray, content);
  QFETCH(bool, created);

  SettingsHolder settingsHolder;

  QTemporaryFile file;
  QVERIFY(file.open());
  QCOMPARE(file.write(content.data(), content.length()), content.length());
  file.close();

  Tutorial* tutorial = Tutorial::create(nullptr, file.fileName());
  QCOMPARE(!!tutorial, created);

  if (!tutorial) {
    return;
  }

  TutorialModel* tm = TutorialModel::instance();
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

  QSignalSpy signalSpy(tm, &TutorialModel::playingChanged);

  tm->play(tutorial);
  QCOMPARE(signalSpy.count(), 1);

  tm->stop();
  QCOMPARE(signalSpy.count(), 2);

  delete tutorial;
}

void TestTutorial::createNotExisting() {
  Tutorial* tutorial = Tutorial::create(nullptr, "aa");
  QVERIFY(!tutorial);
  delete tutorial;
}

static TestTutorial s_testTutorial;
