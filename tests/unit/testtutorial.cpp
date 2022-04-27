/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testtutorial.h"
#include "../../src/models/tutorial.h"
#include "../../src/models/tutorialmodel.h"
#include "../../src/qmlengineholder.h"
#include "../../translations/generated/l18nstrings.h"

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
  QTest::addRow("no-image") << QStringList{"TutorialFooTitle"}
                            << QJsonDocument(obj).toJson() << false;

  obj["image"] = "foo.png";
  QTest::addRow("no-steps") << QStringList{"TutorialFooTitle"}
                            << QJsonDocument(obj).toJson() << false;

  QJsonArray steps;
  obj["steps"] = steps;
  QTest::addRow("with-steps") << QStringList{"TutorialFooTitle"}
                              << QJsonDocument(obj).toJson() << false;

  steps.append("");
  obj["steps"] = steps;
  QTest::addRow("with-invalid-step") << QStringList{"TutorialFooTitle"}
                                     << QJsonDocument(obj).toJson() << false;

  QJsonObject step;
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-id") << QStringList{"TutorialFooTitle"}
                                        << QJsonDocument(obj).toJson() << false;

  step["id"] = "s1";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-without-element")
      << QStringList{"TutorialFooTitle"} << QJsonDocument(obj).toJson()
      << false;

  step["element"] = "wow";
  steps.replace(0, step);
  obj["steps"] = steps;
  QTest::addRow("with-step-element")
      << QStringList{"TutorialFooTitle", "TutorialFooStepS1"}
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

  QVERIFY(!tutorial->isPlaying());

  QString tutorialId = tutorial->property("id").toString();
  QVERIFY(l18nStrings->contains(tutorialId));

  QCOMPARE(tutorial->property("image").toString(), "foo.png");

  QmlEngineHolder qml;

  QSignalSpy signalSpy(tutorial, &Tutorial::playingChanged);

  tutorial->play();
  QCOMPARE(signalSpy.count(), 1);

  tutorial->stop();
  QCOMPARE(signalSpy.count(), 2);

  delete tutorial;
}

void TestTutorial::createNotExisting() {
  Tutorial* tutorial = Tutorial::create(nullptr, "aa");
  QVERIFY(!tutorial);
  delete tutorial;
}

void TestTutorial::model() {
  L18nStrings* l18nStrings = L18nStrings::instance();
  l18nStrings->insert("TutorialFooStepS2", "wow2");

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

static TestTutorial s_testTutorial;
