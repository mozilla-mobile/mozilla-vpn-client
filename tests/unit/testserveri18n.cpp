/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testserveri18n.h"
#include "../../src/serveri18n.h"
#include "../../src/settingsholder.h"

void TestServerI18n::basic() {
  SettingsHolder settingsHolder;

  // Non existing countries/cities
  QCOMPARE(ServerI18N::translateCountryName("FOO", "FOO"), "FOO");
  QCOMPARE(ServerI18N::translateCityName("FOO", "FOO"), "FOO");
  QCOMPARE(ServerI18N::translateCityName("au", "FOO"), "FOO");

  // Existing language
  settingsHolder.setLanguageCode("sk");
  QCOMPARE(ServerI18N::translateCountryName("au", "FOO"), "au_SK");
  QCOMPARE(ServerI18N::translateCityName("au", "Melbourne"), "Melbourne_SK");
  QCOMPARE(ServerI18N::translateCityName("au", "Sydney"), "Sydney_SK");

  // Non-existing language with fallback to en
  settingsHolder.setLanguageCode("fr");
  QCOMPARE(ServerI18N::translateCountryName("au", "FOO"), "au_EN");
  QCOMPARE(ServerI18N::translateCityName("au", "Melbourne"), "Melbourne");
  QCOMPARE(ServerI18N::translateCityName("au", "Sydney"), "Sydney_EN");
}

static TestServerI18n s_testServerI18n;
