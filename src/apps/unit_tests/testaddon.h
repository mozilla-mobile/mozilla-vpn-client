/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class SettingsHolder;

class TestAddon final : public TestHelper {
  Q_OBJECT

 private slots:
  void init();
  void cleanup();

  void property();
  void property_list();

  void conditions_data();
  void conditions();

  void conditionWatcher_locale();
  void conditionWatcher_featuresEnabled();
  void conditionWatcher_group();
  void conditionWatcher_triggerTime();
  void conditionWatcher_startTime();
  void conditionWatcher_endTime();
  void conditionWatcher_javascript();

  void guide_create_data();
  void guide_create();

  void tutorial_create_data();
  void tutorial_create();

  void message_create_data();
  void message_create();
  void message_load_status_data();
  void message_load_status();
  void message_dismiss();

  void telemetry_status_change();

 private:
  SettingsHolder* m_settingsHolder = nullptr;
};
