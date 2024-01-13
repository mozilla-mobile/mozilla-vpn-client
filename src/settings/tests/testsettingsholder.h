/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>
#include <QTest>

#include "settingsholder.h"

class TestSettingsHolder final : public QObject {
  Q_OBJECT

#define SETTING(type, toType, getter, setter, remover, has, ...) \
  void testGetSetCheckRemove_##getter();

#include "settingslist.h"
#undef SETTING

#define EXPERIMENTAL_FEATURE(experimentId, ...) \
  void testGetSet_##experimentId();

#include "feature/experimentalfeaturelist.h"
#undef EXPERIMENTAL_FEATURE

 private slots:
  void runAllTests() {
#define SETTING(type, toType, getter, setter, remover, has, ...) \
  qDebug() << "Testing setting" << #getter;                      \
  testGetSetCheckRemove_##getter();

#include "settingslist.h"
#undef SETTING

#define EXPERIMENTAL_FEATURE(experimentId, ...)                      \
  qDebug() << "Testing experimental setting group" << #experimentId; \
  testGetSet_##experimentId();

#include "feature/experimentalfeaturelist.h"
#undef EXPERIMENTAL_FEATURE
  }
};
