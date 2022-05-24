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
  Localizer l;

  l.setCode("");
  QCOMPARE(l.code(), "");
  QCOMPARE(l.previousCode(), "en");

  l.setCode("en");
  QCOMPARE(l.code(), "en");
  QCOMPARE(l.previousCode(), "en");

  l.setCode("");
  QCOMPARE(l.code(), "");
  QCOMPARE(l.previousCode(), "en");
}

static TestLocalizer s_testLocalizer;
