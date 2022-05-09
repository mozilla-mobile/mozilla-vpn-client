/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtutorial.h"
#include "../../src/models/tutorial.h"
#include "../../src/models/tutorialmodel.h"
#include "../../src/qmlengineholder.h"
#include "../../src/settingsholder.h"
#include "../../translations/generated/l18nstrings.h"

void TestTutorial::model() {
  L18nStrings* l18nStrings = L18nStrings::instance();
  l18nStrings->insert("TutorialDemoTitle", "title");
  l18nStrings->insert("TutorialDemoSubtitle", "subtitle");
  l18nStrings->insert("TutorialDemoCompletionMessage", "completion_message");
  l18nStrings->insert("TutorialDemoStepS1", "wow1");
  l18nStrings->insert("TutorialDemoStepS2", "wow2");

  TutorialModel* mg = TutorialModel::instance();
  QVERIFY(!!mg);

  QHash<int, QByteArray> rn = mg->roleNames();
  QCOMPARE(rn.count(), 1);
  QCOMPARE(rn[TutorialModel::TutorialRole], "tutorial");

  QCOMPARE(mg->rowCount(QModelIndex()), 1);
  QCOMPARE(mg->data(QModelIndex(), TutorialModel::TutorialRole), QVariant());

  Tutorial* tutorial =
      mg->data(mg->index(0, 0), TutorialModel::TutorialRole).value<Tutorial*>();
  QVERIFY(!!tutorial);
}

void TestTutorial::create_data() {
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
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage"}
      << QJsonDocument(obj).toJson() << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-steps")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage"}
      << QJsonDocument(obj).toJson() << false;

  QJsonArray steps;
  obj["steps"] = steps;
  QTest::addRow("with-steps")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage"}
      << QJsonDocument(obj).toJson() << false;

  steps.append("");
  obj["steps"] = steps;
  QTest::addRow("with-invalid-step")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage"}
      << QJsonDocument(obj).toJson() << false;

  QJsonObject step;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-id")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage"}
      << QJsonDocument(obj).toJson() << false;

  step["id"] = "s1";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-element")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage"}
      << QJsonDocument(obj).toJson() << false;

  step["element"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-next")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  step["next"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  QJsonObject nextObj;

  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-1")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  nextObj["op"] = "wow";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-2")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  nextObj["op"] = "signal";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-3")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  nextObj["signal"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-4")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  nextObj["qml_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-5")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << true;

  nextObj["vpn_emitter"] = "a";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-6")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  nextObj.remove("qml_emitter");
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-7")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << false;

  nextObj["vpn_emitter"] = "settingsHolder";
  step["next"] = nextObj;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-with-invalid-next-8")
      << QStringList{"TutorialFooTitle", "TutorialFooSubtitle",
                     "TutorialFooCompletionMessage", "TutorialFooStepS1"}
      << QJsonDocument(obj).toJson() << true;

  obj["conditions"] = QJsonObject();
  QTest::addRow("with-step-element and conditions")
      << QStringList{"GuideFooTitle", "GuideFooBlockA", "GuideFooBlockASub"}
      << QJsonDocument(obj).toJson() << true;
}

void TestTutorial::create() {
  QFETCH(QStringList, l18n);
  QFETCH(QByteArray, content);
  QFETCH(bool, created);

  SettingsHolder settingsHolder;

  L18nStrings* l18nStrings = L18nStrings::instance();
  QVERIFY(!!l18nStrings);
  for (const QString& s : l18n) {
    l18nStrings->insert(s, "WOW!");
  }

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
  QVERIFY(l18nStrings->contains(tutorialTitleId));

  QString tutorialSubtitleId = tutorial->property("subtitleId").toString();
  QVERIFY(l18nStrings->contains(tutorialSubtitleId));

  QString tutorialCompletionMessageId =
      tutorial->property("completionMessageId").toString();
  QVERIFY(l18nStrings->contains(tutorialCompletionMessageId));

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
