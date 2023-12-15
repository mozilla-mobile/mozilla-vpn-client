/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"
#include "settingsholder.h"

class TestSettingsHolder final : public TestHelper {
  Q_OBJECT

 private slots:
#define SETTING(type, toType, getter, setter, remover, has, ...) \
  void testGetSetCheckRemove_##getter();

#include "settingslist.h"
#undef SETTING

#define EXPERIMENTAL_FEATURE(experimentId, ...) \
  void testGetSet_##experimentId();

#include "experimentalfeaturelist.h"
#undef EXPERIMENTAL_FEATURE
};
