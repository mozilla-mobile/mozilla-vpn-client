/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testserveri18n.h"

#include "serveri18n.h"
#include "settings/settingsholder.h"
#include "translations/localizer.h"

void TestServerI18n::basic() {
  SettingsHolder settingsHolder;
  Localizer l;

  // Non existing countries/cities
  QCOMPARE(ServerI18N::instance()->translateCountryName("FOO", "FOO"), "FOO");
  QCOMPARE(ServerI18N::instance()->translateCityName("FOO", "FOO"), "FOO");
  QCOMPARE(ServerI18N::instance()->translateCityName("au", "FOO"), "FOO");

  // Existing language
  settingsHolder.setLanguageCode("sk");
  QCOMPARE(ServerI18N::instance()->translateCountryName("au", "FOO"), "au_SK");
  QCOMPARE(ServerI18N::instance()->translateCityName("au", "Melbourne"),
           "Melbourne_SK");
  QCOMPARE(ServerI18N::instance()->translateCityName("au", "Sydney"),
           "Sydney_SK");

  // Non-existing language with fallback to en
  settingsHolder.setLanguageCode("fr");
  QCOMPARE(ServerI18N::instance()->translateCountryName("au", "FOO"), "au_EN");
  QCOMPARE(ServerI18N::instance()->translateCityName("au", "Melbourne"),
           "Melbourne");
  QCOMPARE(ServerI18N::instance()->translateCityName("au", "Sydney"),
           "Sydney_EN");
}

QTEST_MAIN(TestServerI18n);
