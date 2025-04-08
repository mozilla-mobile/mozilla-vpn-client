/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testthemes.h"

#include <QQmlApplicationEngine>

#include "localizer.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "theme.h"

void TestThemes::loadTheme_data() {
  QTest::addColumn<QString>("theme");
  QTest::addColumn<QString>("expected");

  QTest::addRow("default") << "" << DEFAULT_THEME;
  QTest::addRow(DEFAULT_THEME) << DEFAULT_THEME << DEFAULT_THEME;
  QTest::addRow("foobar") << "foobar"
                          << "foobar";
  QTest::addRow("invalid_theme") << "invalid_theme" << DEFAULT_THEME;
  QTest::addRow("invalid_colors") << "invalid_colors" << DEFAULT_THEME;
  QTest::addRow("error_theme") << "error_theme" << DEFAULT_THEME;
  QTest::addRow("error_colors") << "error_colors" << DEFAULT_THEME;
}

void TestThemes::loadTheme() {
  QFETCH(QString, theme);
  QFETCH(QString, expected);

  SettingsHolder settingsHolder;
  if (!theme.isEmpty()) {
    settingsHolder.setUsingSystemTheme(false);
    settingsHolder.setTheme(theme);
  }

  Localizer l;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  Theme* t = Theme::instance();
  t->initialize(qml.engine());

  QCOMPARE(t->currentTheme(), expected);
  QVERIFY(t->readTheme().isObject());
  QVERIFY(t->readColors().isObject());

  t->setCurrentTheme(theme);
  QCOMPARE(t->currentTheme(), expected);
  QVERIFY(t->readTheme().isObject());
  QVERIFY(t->readColors().isObject());
}

void TestThemes::model() {
  SettingsHolder settingsHolder;
  Localizer l;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  Theme* t = Theme::instance();
  t->initialize(qml.engine());

  QHash<int, QByteArray> rn = t->roleNames();
  QCOMPARE(rn.count(), 1);
  QCOMPARE(rn[Theme::NameRole], "name");

  QCOMPARE(t->rowCount(QModelIndex()), 2 /* main and foobar */);
  QCOMPARE(t->data(QModelIndex(), Theme::NameRole), QVariant());

  QCOMPARE(t->data(t->index(0, 0), Theme::NameRole), "foobar");
  QCOMPARE(t->data(t->index(1, 0), Theme::NameRole), "main");
}

static TestThemes s_testThemes;
