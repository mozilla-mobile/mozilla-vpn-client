/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddonindex.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>

#include "addons/manager/addondirectory.h"
#include "addons/manager/addonindex.h"
#include "feature/feature.h"
#include "settings/settingsholder.h"

void TestAddonIndex::update_data() {
  QTest::addColumn<QByteArray>("index");
  QTest::addColumn<QStringList>("expectedAddonIds");
  QTest::addColumn<bool>("expectsSignal");

  QTest::addRow("invalid") << QByteArray() << QStringList() << false;

  QJsonObject obj;
  QTest::addRow("empty object")
      << QJsonDocument(obj).toJson() << QStringList() << false;

  obj["api_version"] = "0";
  obj["addons"] = QJsonArray();
  QTest::addRow("invalid API version")
      << QJsonDocument(obj).toJson() << QStringList() << false;

  obj["api_version"] = "0.1";
  QTest::addRow("empty") << QJsonDocument(obj).toJson() << QStringList()
                         << true;

  QJsonObject addon;
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("invalid addon")
      << QJsonDocument(obj).toJson() << QStringList() << false;

  addon["sha256"] = "aaa";
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("invalid addon sha256")
      << QJsonDocument(obj).toJson() << QStringList() << false;

  addon["sha256"] =
      "142296a59cf2ef0d56086aca7d756a8424298af4fb3f236a36d5f263fd06fb0a";
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("missing addon id")
      << QJsonDocument(obj).toJson() << QStringList() << false;

  addon["id"] = "foo";
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("one addon")
      << QJsonDocument(obj).toJson() << QStringList{"foo"} << true;

  QJsonObject addon2;
  addon2["id"] = "foobar";
  addon2["sha256"] =
      "142296a59cf2ef0d56086aca7d756a8424298af4fb3f236a36d5f263fd06fb0a";
  obj["addons"] = QJsonArray{addon, addon2};
  QTest::addRow("two addons")
      << QJsonDocument(obj).toJson() << QStringList{"foo", "foobar"} << true;

  obj["addons"] = QJsonArray{addon};
  QTest::addRow("back to one addon")
      << QJsonDocument(obj).toJson() << QStringList{"foo"} << true;
}

void TestAddonIndex::update() {
  QFETCH(QByteArray, index);
  QFETCH(QStringList, expectedAddonIds);
  QFETCH(bool, expectsSignal);

  SettingsHolder settingsHolder;

  settingsHolder.setFeaturesFlippedOff(QStringList{"addonSignature"});

  // This is a horrible hack! The `Feature` objects are created at the startup
  // of the test app, and they listen for signals emitted by another
  // `SettingsHolder`. This means that the previous line does not reset the
  // feature-state. We need to force the feature to read the settings from the
  // current `SettingsHolder`.
  const_cast<Feature*>(Feature::get(Feature::Feature_addonSignature))
      ->maybeFlipOnOrOff();

  AddonDirectory ad;
  AddonIndex ai(&ad);

  QSignalSpy indexUpdatedSpy(&ai, SIGNAL(indexUpdated(bool, QList<AddonData>)));

  ai.update(index, QByteArray("test signature"));
  QTRY_COMPARE(indexUpdatedSpy.count(), 1);

  if (expectsSignal) {
    QList<QVariant> arguments = indexUpdatedSpy.takeFirst();

    QVERIFY(arguments.at(0).toBool());
    QList<AddonData> addonData = arguments.at(1).value<QList<AddonData>>();

    QStringList addonIds;
    for (int i = 0; i < addonData.size(); ++i) {
      addonIds.append(addonData.at(i).m_addonId);
    }

    QCOMPARE(addonIds, expectedAddonIds);
  }
}

void TestAddonIndex::testSignatureChecksCanBeToggled() {
  SettingsHolder settingsHolder;

  settingsHolder.setFeaturesFlippedOff(QStringList{"addonSignature"});

  // This is a horrible hack! The `Feature` objects are created at the startup
  // of the test app, and they listen for signals emitted by another
  // `SettingsHolder`. This means that the previous line does not reset the
  // feature-state. We need to force the feature to read the settings from the
  // current `SettingsHolder`.
  const_cast<Feature*>(Feature::get(Feature::Feature_addonSignature))
      ->maybeFlipOnOrOff();

  AddonDirectory ad;
  AddonIndex ai(&ad);

  QSignalSpy indexUpdatedSpy(&ai, SIGNAL(indexUpdated(bool, QList<AddonData>)));

  QJsonObject addon;
  addon["sha256"] =
      "142296a59cf2ef0d56086aca7d756a8424298af4fb3f236a36d5f263fd06fb0a";
  addon["id"] = "foo";

  QJsonObject index;
  index["api_version"] = "0.1";
  index["addons"] = QJsonArray{addon};

  // We need to reset otherwise update
  // will bail early due to index not having changed.
  ad.reset();
  ai.update(QJsonDocument(index).toJson(), QByteArray());
  QTRY_COMPARE(indexUpdatedSpy.count(), 1);

  settingsHolder.setFeaturesFlippedOn(QStringList{"addonSignature"});
  const_cast<Feature*>(Feature::get(Feature::Feature_addonSignature))
      ->maybeFlipOnOrOff();

  // We need to reset otherwise update
  // will bail early due to index not having changed.
  ad.reset();
  ai.update(QJsonDocument(index).toJson(), QByteArray());

  // The update has triggered a second signal.
  QTRY_COMPARE(indexUpdatedSpy.count(), 2);

  // With a non empty signature all is good.
  ai.update(QJsonDocument(index).toJson(), QByteArray("test signature"));
  QTRY_COMPARE(indexUpdatedSpy.count(), 3);
}

QTEST_MAIN(TestAddonIndex)
