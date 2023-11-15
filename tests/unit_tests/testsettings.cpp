/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsettings.h"

#include "helper.h"
#include "settingsholder.h"

void TestSettings::sensitiveLogging() {
  SettingsHolder settingsHolder;
  settingsHolder.setFoobar("NotSensitive");
  settingsHolder.setSensitive("Do NOT print this out!");

  QString report;
  // We can do this because SettingsHolder::serializeLogs() is sync.
  settingsHolder.serializeLogs(
      [report = &report](const QString& name, const QString& logs) {
        *report = logs;
      });

  QVERIFY(report.contains("\nfoobar -> NotSensitive\n"));
  QVERIFY(report.contains("\nsensitive -> <Sensitive>\n"));
  QVERIFY(!report.contains("Do NOT print this out!"));
}

static TestSettings s_testSettings;
