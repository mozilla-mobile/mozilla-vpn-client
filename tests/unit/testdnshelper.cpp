/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdnshelper.h"

#include "dnshelper.h"
#include "settingsholder.h"

void TestDNSHelper::cleanup() { SettingsHolder::testCleanup(); }

void TestDNSHelper::migration_data() {
  QTest::addColumn<int>("deprecated");
  QTest::addColumn<int>("flags");

  QTest::addRow("gateway") << 0 << static_cast<int>(SettingsHolder::Gateway);
  QTest::addRow("block all") << 1
                             << static_cast<int>(SettingsHolder::BlockAds &
                                                 SettingsHolder::BlockTrackers);
  QTest::addRow("block ads") << 2 << static_cast<int>(SettingsHolder::BlockAds);
  QTest::addRow("block trackers")
      << 3 << static_cast<int>(SettingsHolder::BlockTrackers);
  QTest::addRow("custom") << 4 << static_cast<int>(SettingsHolder::Custom);
  QTest::addRow("invalid") << 42 << static_cast<int>(SettingsHolder::Gateway);
}

void TestDNSHelper::migration() {
  qDebug() << SettingsHolder::instance()->hasDNSProviderFlags();

  QVERIFY(!SettingsHolder::instance()->hasDNSProviderFlags());
  QVERIFY(!SettingsHolder::instance()->hasDNSProviderDeprecated());

  QFETCH(int, deprecated);
  qDebug() << "Setting deprecated to" << deprecated;
  SettingsHolder::instance()->setDNSProviderDeprecated(deprecated);
  QVERIFY(SettingsHolder::instance()->hasDNSProviderDeprecated());
  DNSHelper::maybeMigrateDNSProviderFlags();
  QVERIFY(!SettingsHolder::instance()->hasDNSProviderDeprecated());

  QFETCH(int, flags);
  QCOMPARE(SettingsHolder::instance()->dnsProviderFlags(), flags);
}

static TestDNSHelper s_testDNSHelper;
