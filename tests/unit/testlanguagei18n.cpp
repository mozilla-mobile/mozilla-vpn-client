/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlanguagei18n.h"

#include "languagei18n.h"
#include "localizer.h"
#include "settingsholder.h"

void TestLanguageI18n::basic() {
  QVERIFY(LanguageI18N::languageExists("tlh"));
  QVERIFY(!LanguageI18N::languageExists("FOO"));

  // Non existing language
  QVERIFY(LanguageI18N::translateLanguage("FOO", "FOO").isEmpty());

  // Self-translation
  QCOMPARE(LanguageI18N::translateLanguage("tlh", "tlh"), " ");

  // Other language
  QCOMPARE(LanguageI18N::translateLanguage("fr", "tlh"), "klingon");

  // Non existing translation
  QVERIFY(LanguageI18N::translateLanguage("fi", "tlh").isEmpty());
}

static TestLanguageI18n s_testLanguageI18n;
