/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlocalizer.h"
#include "../../src/localizer.h"
#include "helper.h"
#include "settingsholder.h"

void TestLocalizer::basic() {
  SettingsHolder settings;
  Localizer l;
  QCOMPARE(Localizer::instance(), &l);

  QHash<int, QByteArray> rn = l.roleNames();
  QCOMPARE(rn.count(), 3);
  QCOMPARE(rn[Localizer::LanguageRole], "language");
  QCOMPARE(rn[Localizer::LocalizedLanguageRole], "localizedLanguage");
  QCOMPARE(rn[Localizer::CodeRole], "code");

  QVERIFY(l.rowCount(QModelIndex()) > 0);
  QCOMPARE(l.data(QModelIndex(), Localizer::LanguageRole), QVariant());
}

void TestLocalizer::systemLanguage() {
  SettingsHolder settings;
  settings.hardReset();

  Localizer l;

  l.setCode("");
  QCOMPARE(l.code(), "");

  l.setCode("en");
  QCOMPARE(l.code(), "en");
  QVERIFY(!l.previousCode().isEmpty());

  l.setCode("");
  QCOMPARE(l.code(), "");
  QCOMPARE(l.previousCode(), "en");

  QVERIFY(!Localizer::systemLanguageCode().isEmpty());
}

void TestLocalizer::localizeCurrency() {
  SettingsHolder settings;
  Localizer l;
  l.setCode("en_GB");

  // Invalid iso4217 values
  QCOMPARE(l.localizeCurrency(123.123, "FOOBAR"), "FOOBAR123.12");
  QCOMPARE(l.localizeCurrency(123.123, "F"), "F123.12");

  // Happy path
  QCOMPARE(l.localizeCurrency(123.123, "GBP"), "£123.12");

  // Let's guess - invalid currency
  QCOMPARE(l.localizeCurrency(123.123, "AAA"), "AAA123.12");

  // Let's guess - valid currency
  QVERIFY(l.localizeCurrency(123.123, "EUR").contains("€"));
}

void TestLocalizer::majorLanguageCode() {
  QCOMPARE(Localizer::majorLanguageCode(""), "");
  QCOMPARE(Localizer::majorLanguageCode("fo"), "fo");
  QCOMPARE(Localizer::majorLanguageCode("fo-BA"), "fo");
  QCOMPARE(Localizer::majorLanguageCode("fo_BA"), "fo");
}

static TestLocalizer s_testLocalizer;
