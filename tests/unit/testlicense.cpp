/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testlicense.h"
#include "../../src/ipfinder.h"
#include "../../src/settingsholder.h"
#include "helper.h"

void TestLicense::parse() {
  LicenseModel lm;

  QCOMPARE(lm.contentLicense().length(), 0);
  QCOMPARE(lm.rowCount(QModelIndex()), 0);

  lm.initialize();
  QVERIFY(lm.contentLicense().length() > 0);
  QVERIFY(lm.rowCount(QModelIndex()) > 0);

  for (int i = 0; i < lm.rowCount(QModelIndex()); ++i) {
    QVERIFY(
        lm.data(lm.index(i, 0), LicenseModel::TitleRole).toString().length() >
        0);
    QVERIFY(
        lm.data(lm.index(i, 0), LicenseModel::ContentRole).toString().length() >
        0);
  }
}

static TestLicense s_testLicense;
