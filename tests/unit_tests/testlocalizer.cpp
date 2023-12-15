/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlocalizer.h"

#include "glean/generated/metrics.h"
#include "glean/mzglean.h"
#include "helper.h"
#include "localizer.h"
#include "qtglean.h"
#include "settings/settingsbase.h"
#include "settingsholder.h"

void TestLocalizer::init() {
  m_settingsHolder = new SettingsHolder();

  // Glean needs to be initialized for every test because this test suite
  // includes telemetry tests.
  //
  // Glean operations are queued and applied once Glean is initialized.
  // If we only initialize it in the test that actually tests telemetry all
  // of the Glean operations from previous tests will be applied and mess with
  // the state of the test that actually is testing telemetry.
  //
  // Note: on tests Glean::initialize clears Glean's storage.
  MZGlean::initialize();
}

void TestLocalizer::cleanup() { delete m_settingsHolder; }

void TestLocalizer::basic() {
  Localizer l;
  QCOMPARE(Localizer::instance(), &l);

  QHash<int, QByteArray> rn = l.roleNames();
  QCOMPARE(rn.count(), 4);
  QCOMPARE(rn[Localizer::LocalizedLanguageNameRole], "localizedLanguageName");
  QCOMPARE(rn[Localizer::NativeLanguageNameRole], "nativeLanguageName");
  QCOMPARE(rn[Localizer::CodeRole], "code");
  QCOMPARE(rn[Localizer::RTLRole], "isRightToLeft");

  QVERIFY(l.rowCount(QModelIndex()) == 4);
  QCOMPARE(l.data(QModelIndex(), Localizer::LocalizedLanguageNameRole),
           QVariant());
}

void testGleanEntries(const QString& language) {
  QCOMPARE(mozilla::glean::sample::non_default_language_used
               .testGetNumRecordedErrors(ErrorType::InvalidValue),
           0);
  QCOMPARE(mozilla::glean::sample::non_default_language_used
               .testGetNumRecordedErrors(ErrorType::InvalidOverflow),
           0);

  auto gleanValues =
      mozilla::glean::sample::non_default_language_used.testGetValue();
  QCOMPARE(gleanValues.length(), language.isEmpty() ? 0 : 1);

  if (!language.isEmpty()) {
    QCOMPARE(gleanValues[0]["extra"].toObject()["language_code"].toString(),
             language);
  }
}

void TestLocalizer::systemLanguage() {
  SettingsBase::hardReset();

  Localizer l;

  m_settingsHolder->setLanguageCode("");
  QCOMPARE(m_settingsHolder->languageCode(), "");
  QCOMPARE(l.languageCodeOrSystem(), "en");
  testGleanEntries(QString());

  m_settingsHolder->setLanguageCode("en");
  QCOMPARE(m_settingsHolder->languageCode(), "en");
  QVERIFY(!m_settingsHolder->previousLanguageCode().isEmpty());
  QCOMPARE(l.languageCodeOrSystem(), "en");
  testGleanEntries("en");

  m_settingsHolder->setLanguageCode("");
  QCOMPARE(m_settingsHolder->languageCode(), "");
  QCOMPARE(m_settingsHolder->previousLanguageCode(), "en");
  QCOMPARE(l.languageCodeOrSystem(), "en");
  testGleanEntries("en");
}

void TestLocalizer::localizeCurrency() {
  Localizer l;
  m_settingsHolder->setLanguageCode("en_GB");
  testGleanEntries("en_GB");

  // Invalid iso4217 values
  QCOMPARE(l.localizeCurrency(123.123, "FOOBAR"), "FOOBAR123.12");
  QCOMPARE(l.localizeCurrency(123.123, "F"), "F123.12");

  // Happy path
  QCOMPARE(l.localizeCurrency(123.123, "GBP"), "£123.12");

  // Let's guess - invalid currency
  QCOMPARE(l.localizeCurrency(123.123, "AAA"), "AAA123.12");

  // Let's guess - valid currency for CLDR
  QCOMPARE(l.localizeCurrency(123.123, "BBB"), "WOW123.12");
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

// QFETCH fails with double templates. Let's use a typedef to make it happy.
typedef QMap<QString, double> CompletenessList;

void TestLocalizer::completeness_data() {
  QTest::addColumn<QString>("filename");
  QTest::addColumn<CompletenessList>("output");

  {
    CompletenessList a;
    QTest::addRow("empty") << ":/i18n/completeness_empty.txt" << a;
  }

  {
    CompletenessList a;
    QTest::addRow("invalid") << ":/i18n/completeness_invalid.txt" << a;
  }

  {
    CompletenessList a{{"tlh", 0.8}, {"foo", 0}, {"bar", 1}};
    QTest::addRow("ok") << ":/i18n/completeness_ok.txt" << a;
  }
}

void TestLocalizer::completeness() {
  QFETCH(QString, filename);
  QFETCH(CompletenessList, output);

  CompletenessList list = Localizer::loadTranslationCompleteness(filename);

  for (const QString& key : output.keys()) {
    QCOMPARE(list.value(key, -1), output.value(key));
  }
}

void TestLocalizer::fallback() {
  SettingsHolder::instance()->setLanguageCode("es_MX");

  Localizer l;

  // We have 3 languages in this test.
  QCOMPARE(l.languages(), QStringList() << "en"
                                        << "es_CL"
                                        << "es_ES"
                                        << "es_MX");

  // MX contains translations for "foo.1"
  QCOMPARE(qtTrId("foo.1"), "hello world 1 es_MX");

  // MX does not have "foo.2", fallback CL
  QCOMPARE(qtTrId("foo.2"), "hello world 2 es_CL");

  // MX and CL do not have "foo.3", fallback ES
  QCOMPARE(qtTrId("foo.3"), "hello world 3 es_ES");

  // No translations for "foo.4", fallback English
  QCOMPARE(qtTrId("foo.4"), "Hello world 4 - fallback");
}

void TestLocalizer::formattedDate_data() {
  QTest::addColumn<QString>("languageCode");
  QTest::addColumn<QDateTime>("now");
  QTest::addColumn<QDateTime>("date");
  QTest::addColumn<QString>("result");
  QTest::addColumn<qint64>("timer");

  QTest::addRow("en - future")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(11, 0), QTimeZone(0)) << "10:00 AM"
      << (qint64)(14 * 3600);
  QTest::addRow("es_ES - future")
      << "es_ES" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(11, 0), QTimeZone(0)) << "10:00"
      << (qint64)(14 * 3600);

  QTest::addRow("en - same")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0)) << "10:00 AM"
      << (qint64)(14 * 3600);
  QTest::addRow("es_ES - same")
      << "es_ES" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0)) << "10:00"
      << (qint64)(14 * 3600);

  QTest::addRow("en - one hour ago")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(9, 0), QTimeZone(0)) << "9:00 AM"
      << (qint64)(15 * 3600);
  QTest::addRow("es_ES - one hour ago")
      << "es_ES" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(9, 0), QTimeZone(0)) << "9:00"
      << (qint64)(15 * 3600);

  QTest::addRow("en - midnight")
      << "en" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(0, 0), QTimeZone(0)) << "12:00 AM"
      << (qint64)(24 * 3600);
  QTest::addRow("es_ES - midnight")
      << "es_ES" << QDateTime(QDate(2000, 1, 1), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(0, 0), QTimeZone(0)) << "0:00"
      << (qint64)(24 * 3600);

  QTest::addRow("en - yesterday but less than 24 hours")
      << "en" << QDateTime(QDate(2000, 1, 2), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(21, 0), QTimeZone(0)) << "Yesterday"
      << (qint64)(3 * 3600);

  QTest::addRow("en - yesterday more than 24 hours")
      << "en" << QDateTime(QDate(2000, 1, 2), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 1), QTime(9, 0), QTimeZone(0)) << "Yesterday"
      << (qint64)-1;

  QTest::addRow("en - 2 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 8), QTime(10, 0), QTimeZone(0)) << "Saturday"
      << (qint64)-1;

  QTest::addRow("en - 3 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 7), QTime(10, 0), QTimeZone(0)) << "Friday"
      << (qint64)-1;

  QTest::addRow("en - 4 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 6), QTime(10, 0), QTimeZone(0)) << "Thursday"
      << (qint64)-1;

  QTest::addRow("en - 5 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 5), QTime(10, 0), QTimeZone(0)) << "Wednesday"
      << (qint64)-1;

  QTest::addRow("en - 6 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 4), QTime(10, 0), QTimeZone(0)) << "Tuesday"
      << (qint64)-1;

  QTest::addRow("en - 7 days ago")
      << "en" << QDateTime(QDate(2000, 1, 10), QTime(10, 0), QTimeZone(0))
      << QDateTime(QDate(2000, 1, 3), QTime(10, 0), QTimeZone(0)) << "1/3/00"
      << (qint64)-1;
}

void TestLocalizer::formattedDate() {
  Localizer l;

  QFETCH(QString, languageCode);
  SettingsHolder::instance()->setLanguageCode(languageCode);

  QFETCH(QDateTime, now);
  QVERIFY(now.isValid());

  QFETCH(QDateTime, date);
  QVERIFY(date.isValid());

  QFETCH(QString, result);
  QCOMPARE(Localizer::instance()->formatDate(now, date, "Yesterday"), result);
}

static TestLocalizer s_testLocalizer;
