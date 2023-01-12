/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlocalizer.h"

#include "helper.h"
#include "localizer.h"
#include "settingsholder.h"

void TestLocalizer::basic() {
  SettingsHolder settings;
  Localizer l;
  QCOMPARE(Localizer::instance(), &l);

  QHash<int, QByteArray> rn = l.roleNames();
  QCOMPARE(rn.count(), 4);
  QCOMPARE(rn[Localizer::LanguageRole], "language");
  QCOMPARE(rn[Localizer::LocalizedLanguageRole], "localizedLanguage");
  QCOMPARE(rn[Localizer::CodeRole], "code");
  QCOMPARE(rn[Localizer::RTLRole], "isRightToLeft");

  QVERIFY(l.rowCount(QModelIndex()) > 0);
  QCOMPARE(l.data(QModelIndex(), Localizer::LanguageRole), QVariant());
}

void TestLocalizer::systemLanguage() {
  SettingsHolder settings;
  settings.hardReset();

  Localizer l;

  settings.setLanguageCode("");
  QCOMPARE(settings.languageCode(), "");
  QCOMPARE(l.languageCodeOrSystem(), "en");

  settings.setLanguageCode("en");
  QCOMPARE(settings.languageCode(), "en");
  QVERIFY(!settings.previousLanguageCode().isEmpty());
  QCOMPARE(l.languageCodeOrSystem(), "en");

  settings.setLanguageCode("");
  QCOMPARE(settings.languageCode(), "");
  QCOMPARE(settings.previousLanguageCode(), "en");
  QCOMPARE(l.languageCodeOrSystem(), "en");
}

void TestLocalizer::localizeCurrency() {
  SettingsHolder settings;
  Localizer l;
  settings.setLanguageCode("en_GB");

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

// QFETCH fails with double templates. Let's use a typedef to make it happy.
typedef QList<QPair<QString, QString>> LanguageList;

void TestLocalizer::parseBCP47Languages_data() {
  QTest::addColumn<QStringList>("input");
  QTest::addColumn<LanguageList>("output");

  QTest::addRow("empty") << QStringList() << QList<QPair<QString, QString>>();

  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", ""});
    QTest::addRow("simple") << QStringList("aa") << a;
  }

  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", "bb"});
    QTest::addRow("language-country") << QStringList("aa-bb") << a;
  }
  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", ""});
    QTest::addRow("language-extension") << QStringList("aa-ccc") << a;
  }
  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", "bb"});
    QTest::addRow("language-extension-country")
        << QStringList("aa-ccc-bb") << a;
  }
  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", "bb"});
    QTest::addRow("language-extension2-country")
        << QStringList("aa-ccc--dd-bb") << a;
  }
}

void TestLocalizer::parseBCP47Languages() {
  QFETCH(QStringList, input);
  QFETCH(LanguageList, output);

  QList<QPair<QString, QString>> list = Localizer::parseBCP47Languages(input);
  QCOMPARE(list.length(), output.length());

  for (int i = 0; i < list.length(); ++i) {
    const QPair<QString, QString>& a = list[i];
    const QPair<QString, QString>& b = output[i];
    QCOMPARE(a.first, b.first);
    QCOMPARE(a.second, b.second);
  }
}

void TestLocalizer::parseIOSLanguages_data() {
  QTest::addColumn<QStringList>("input");
  QTest::addColumn<LanguageList>("output");

  QTest::addRow("empty") << QStringList() << QList<QPair<QString, QString>>();

  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", ""});
    QTest::addRow("simple") << QStringList("aa") << a;
  }

  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", "bb"});
    QTest::addRow("language_country") << QStringList("aa_bb") << a;
  }
  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", ""});
    QTest::addRow("language-script") << QStringList("aa-cc") << a;
  }
  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", "bb"});
    QTest::addRow("language-script_country") << QStringList("aa-cc_bb") << a;
  }
}

void TestLocalizer::parseIOSLanguages() {
  QFETCH(QStringList, input);
  QFETCH(LanguageList, output);

  QList<QPair<QString, QString>> list = Localizer::parseIOSLanguages(input);
  QCOMPARE(list.length(), output.length());

  for (int i = 0; i < list.length(); ++i) {
    const QPair<QString, QString>& a = list[i];
    const QPair<QString, QString>& b = output[i];
    QCOMPARE(a.first, b.first);
    QCOMPARE(a.second, b.second);
  }
}

static TestLocalizer s_testLocalizer;
