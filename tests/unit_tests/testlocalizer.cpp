/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlocalizer.h"

#include "glean/generated/metrics.h"
#include "glean/mzglean.h"
#include "helper.h"
#include "localizer.h"
#include "qtglean.h"
#include "settings/settingsmanager.h"
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
  MZGlean::initialize("testing");
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

  QVERIFY(l.rowCount(QModelIndex()) == 5);
  QCOMPARE(l.data(QModelIndex(), Localizer::LocalizedLanguageNameRole),
           QVariant());
}

void TestLocalizer::systemLanguage() {
  SettingsManager::instance()->hardReset();

  Localizer l;

  m_settingsHolder->setLanguageCode("");
  QCOMPARE(m_settingsHolder->languageCode(), "");
  QCOMPARE(l.languageCodeOrSystem(), "en");

  m_settingsHolder->setLanguageCode("en");
  QCOMPARE(m_settingsHolder->languageCode(), "en");
  QVERIFY(!m_settingsHolder->previousLanguageCode().isEmpty());
  QCOMPARE(l.languageCodeOrSystem(), "en");

  m_settingsHolder->setLanguageCode("");
  QCOMPARE(m_settingsHolder->languageCode(), "");
  QCOMPARE(m_settingsHolder->previousLanguageCode(), "en");
  QCOMPARE(l.languageCodeOrSystem(), "en");
}

void TestLocalizer::localizeCurrency() {
  Localizer l;
  m_settingsHolder->setLanguageCode("en_GB");

  // Invalid iso4217 values
  QCOMPARE(l.localizeCurrency(123.123, "FOOBAR"), "FOOBAR123.12");
  QCOMPARE(l.localizeCurrency(123.123, "F"), "F123.12");

  // Happy path
  QCOMPARE(l.localizeCurrency(123.123, "GBP"), "£123.12");
  QCOMPARE(l.localizeCurrency(123.123, "USD"), "$123.12");
  QCOMPARE(l.localizeCurrency(123.123, "BGN"), "лв123.12");

  // Let's guess - invalid currency
  QCOMPARE(l.localizeCurrency(123.123, "AAA"), "AAA123.12");
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
    QTest::addRow("language-country") << QStringList("aa-bb") << a;
  }
  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", "cc"});
    QTest::addRow("language-script") << QStringList("aa-cc") << a;
  }
  {
    LanguageList a;
    a.append(QPair<QString, QString>{"aa", "cc"});
    QTest::addRow("language-script-country") << QStringList("aa-cc-bb") << a;
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
                                        << "es_MX"
                                        << "pt_BR");

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

  /*

    CLDR (Common Locale Data Repository) 42 changed the formatting here
    from a white space to a Narrow No-Break Space. Qt adopted CLDR 42 in Qt6.5.
    This means that tests run with Qt6.2.4 should expect a white space,
    and tests run with Qt6.6 should expect a Narrow No-Break Space (U+202F).
    We can remove this workaround once all the tests are running on 6.6

    CLDR change: https://unicode-org.atlassian.net/browse/CLDR-14032
    Qt6.5 CLDR change note: https://doc.qt.io/qt-6/license-changes.html#qt-6-5-0

  */

  QString expectedString =
      Localizer::instance()->formatDate(now, date, "Yesterday");
  QCOMPARE(expectedString.replace(" ", " "), result);
}

void TestLocalizer::nativeLanguageName_data() {
  QTest::addColumn<QLocale>("locale");
  QTest::addColumn<QString>("code");
  QTest::addColumn<QString>("output");

  // Happy path
  QTest::addRow("existing transation") << QLocale(QLocale::C) << "en"
                                       << "English";

  // Happy path
  QTest::addRow("existing transation") << QLocale(QLocale::Spanish) << "es_ES"
                                       << "Español de españa";

  // There is a key for this locale in NATIVE_LANGUAGE_NAMES,
  // but there is no translation. This is going to use Qt's translation
  // (which in this case is a terrible translation).
  QTest::addRow("missing transation") << QLocale(QLocale::Spanish) << "es_CL"
                                      << "Español de España";

  // When the locale code provided is not in the map,
  // but the QLocale provided is QLocale::C we will fallback to en_US.
  QTest::addRow("english fallback") << QLocale(QLocale::C) << "Unknown"
                                    << "English (US)";

  // When all else fails, we just trust Qt again.
  QTest::addRow("qt fallback") << QLocale(QLocale::Portuguese) << "Unknown"
                               << "Português";
}

void TestLocalizer::nativeLanguageName() {
  QFETCH(QLocale, locale);
  QFETCH(QString, code);
  QFETCH(QString, output);

  QCOMPARE(Localizer::nativeLanguageName(locale, code), output);
}

void TestLocalizer::localizedLanguageName() {
  Localizer localizer;

  // Happy path
  QCOMPARE(localizer.localizedLanguageName("en"), "English");

  m_settingsHolder->setLanguageCode("es_ES");
  QCOMPARE(localizer.localizedLanguageName("en"), "Inglés");

  // This language doesn't have a translations for "es_MX".
  // It does have fallback into es_MX and then es_ES.
  // It should get the translation from the first fallback language: es_MX.
  m_settingsHolder->setLanguageCode("es_CL");
  QCOMPARE(localizer.localizedLanguageName("es_MX"), "Español");

  // This language doesn't have a translations for "es_ES",
  // and neither does the first fallback language: es_MX.
  // It should still get the translation from the second fallback language
  QCOMPARE(localizer.localizedLanguageName("es_ES"), "Español de españa");

  // This language doesn't have a translations for "es_ES"
  // and it also doesn't have fallbacks. "en" is the default fallback locale.
  m_settingsHolder->setLanguageCode("pt_BR");
  QCOMPARE(localizer.localizedLanguageName("es_ES"), "Spanish");
}

void TestLocalizer::getTranslatedCountryName() {
  Localizer localizer;

  // Make sure we don't crash on an empty input.
  QCOMPARE(localizer.getTranslatedCountryName("", ""), "");

  // If the country name provided doesn't match any translation,
  // just return what was provided.
  QCOMPARE(localizer.getTranslatedCountryName("Middle Earth", "Middle Earth"),
           "Middle Earth");

  // Happy path
  QCOMPARE(localizer.getTranslatedCountryName("us", ""),
           "United States of America");

  m_settingsHolder->setLanguageCode("es_ES");
  QCOMPARE(localizer.getTranslatedCountryName("us", ""),
           "Estados Unidos de América");

  // This language doesn't have a translations for "us",
  // but it should not fallback to English. It should fallback to Spanish.
  m_settingsHolder->setLanguageCode("es_CL");
  QCOMPARE(localizer.getTranslatedCountryName("us", ""),
           "Estados Unidos de América");

  // This language doesn't have a translations for "en"
  // and it also doesn't have fallbacks. It should fallback to English.
  m_settingsHolder->setLanguageCode("pt_BR");
  QCOMPARE(localizer.getTranslatedCountryName("us", ""),
           "United States of America");
}

void TestLocalizer::getTranslatedCityName() {
  Localizer localizer;

  // Make sure we don't crash on an empty input.
  QCOMPARE(localizer.getTranslatedCityName(""), "");

  // If the city name provided doesn't match any translation,
  // just return what was provided.
  QCOMPARE(localizer.getTranslatedCityName("Hogsmead"), "Hogsmead");

  // Make sure special characters and state suffixes don't trip us up.
  QCOMPARE(localizer.getTranslatedCityName("Salt Lake City, UT"),
           "Salt Lake City");
  QCOMPARE(localizer.getTranslatedCityName("São Paulo, SP"), "São Paulo");

  // Happy path
  m_settingsHolder->setLanguageCode("es_ES");
  QCOMPARE(localizer.getTranslatedCityName("Mexico City"), "Ciudad de México");

  // This language doesn't have a translations for "Mexico City",
  // but it should not fallback to English. It should fallback to Spanish.
  m_settingsHolder->setLanguageCode("es_CL");
  QCOMPARE(localizer.getTranslatedCityName("Mexico City"), "Ciudad de México");

  // This language doesn't have a translations for "en"
  // and it also doesn't have fallbacks. It should fallback to English.
  m_settingsHolder->setLanguageCode("pt_BR");
  QCOMPARE(localizer.getTranslatedCityName("Mexico City"), "Mexico City");
}

static TestLocalizer s_testLocalizer;
