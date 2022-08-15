/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddonindex.h"
#include "../../src/addons/manager/addondirectory.h"
#include "../../src/addons/manager/addonindex.h"
#include "../../src/settingsholder.h"

void TestAddonIndex::update_data() {
  QTest::addColumn<QByteArray>("index");
  QTest::addColumn<QStringList>("expectedAddonIds");

  QTest::addRow("invalid") << QByteArray() << QStringList();

  QJsonObject obj;
  QTest::addRow("empty object") << QJsonDocument(obj).toJson() << QStringList();

  obj["api_version"] = "0";
  obj["addons"] = QJsonArray();
  QTest::addRow("invalid API version")
      << QJsonDocument(obj).toJson() << QStringList();

  obj["api_version"] = "0.1";
  QTest::addRow("empty") << QJsonDocument(obj).toJson() << QStringList();

  QJsonObject addon;
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("invalid addon")
      << QJsonDocument(obj).toJson() << QStringList();

  addon["sha256"] = "aaa";
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("invalid addon sha256")
      << QJsonDocument(obj).toJson() << QStringList();

  addon["sha256"] =
      "142296a59cf2ef0d56086aca7d756a8424298af4fb3f236a36d5f263fd06fb0a";
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("missing addon id")
      << QJsonDocument(obj).toJson() << QStringList();

  addon["id"] = "foo";
  obj["addons"] = QJsonArray{addon};
  QTest::addRow("one addon")
      << QJsonDocument(obj).toJson() << QStringList{"foo"};

  QJsonObject addon2;
  addon2["id"] = "foobar";
  addon2["sha256"] =
      "142296a59cf2ef0d56086aca7d756a8424298af4fb3f236a36d5f263fd06fb0a";
  obj["addons"] = QJsonArray{addon, addon2};
  QTest::addRow("two addons")
      << QJsonDocument(obj).toJson() << QStringList{"foo", "foobar"};

  obj["addons"] = QJsonArray{addon};
  QTest::addRow("back to one addon")
      << QJsonDocument(obj).toJson() << QStringList{"foo"};
}

void TestAddonIndex::update() {
  QFETCH(QByteArray, index);
  QFETCH(QStringList, expectedAddonIds);

  SettingsHolder settingsHolder;

  AddonDirectory ad;
  AddonIndex ai(&ad);

  QSignalSpy indexUpdatedSpy(&ai, SIGNAL(indexUpdated(QList<AddonData>)));

  ai.update(index, QByteArray("test signature"));

  if (expectedAddonIds.isEmpty()) {
    QTRY_COMPARE(indexUpdatedSpy.count(), 0);
  } else {
    QTRY_COMPARE(indexUpdatedSpy.count(), 1);

    QList<QVariant> arguments = indexUpdatedSpy.takeFirst();
    QList<AddonData> addonData = arguments.at(0).value<QList<AddonData>>();

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

  AddonDirectory ad;
  AddonIndex ai(&ad);

  QSignalSpy indexUpdatedSpy(&ai, SIGNAL(indexUpdated(QList<AddonData>)));

  QJsonObject addon;
  addon["sha256"] =
      "142296a59cf2ef0d56086aca7d756a8424298af4fb3f236a36d5f263fd06fb0a";
  addon["id"] = "foo";

  QJsonObject index;
  index["api_version"] = "0.1";
  index["addons"] = QJsonArray{addon};

  // We need to reset otherwise update
  // will bail early due to index not having changed.
  ad.testReset();
  ai.update(QJsonDocument(index).toJson(), nullptr);

  QTRY_COMPARE(indexUpdatedSpy.count(), 1);

  settingsHolder.setFeaturesFlippedOn(QStringList{"addonSignature"});

  // We need to reset otherwise update
  // will bail early due to index not having changed.
  ad.testReset();
  ai.update(QJsonDocument(index).toJson(), nullptr);

  // Still one, no signals emitted. Validation should have failed.
  QTRY_COMPARE(indexUpdatedSpy.count(), 1);

  // With a non null signature all is good.
  ai.update(QJsonDocument(index).toJson(), QByteArray("test signature"));
  QTRY_COMPARE(indexUpdatedSpy.count(), 2);
}

static TestAddonIndex s_testAddonIndex;
