/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class SettingsHolder;

class TestNavigator final : public TestHelper {
  Q_OBJECT

 private slots:
  void init();
  void cleanup();

  void testNavbarButtonTelemetry();
  void testNavbarButtonTelemetryNoLayers();

 private:
  SettingsHolder* m_settingsHolder = nullptr;
};
