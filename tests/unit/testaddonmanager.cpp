/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddonmanager.h"
#include "../../src/addonmanager.h"
#include "../../src/settingsholder.h"

void TestAddonManager::index_data() {
  QTest::addColumn<QByteArray>("index");
  QTest::addColumn<QStringList>("addonIds");

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

void TestAddonManager::index() {
  QFETCH(QByteArray, index);
  QFETCH(QStringList, addonIds);

  SettingsHolder settingsHolder;

  AddonManager* am = AddonManager::instance();
  QVERIFY(!!am);

  for (int i = 0; i < addonIds.length(); ++i) {
    TestHelper::networkConfig.append(TestHelper::NetworkConfig(
        TestHelper::NetworkConfig::Failure, QByteArray()));
  }

  am->updateIndex(index, QByteArray());
  QCOMPARE(addonIds, am->addonIds());
}

static TestAddonManager s_testAddonManager;
