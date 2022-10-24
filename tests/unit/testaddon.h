/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestAddon final : public TestHelper {
  Q_OBJECT

 private slots:
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
  void message_date_data();
  void message_date();
  void message_notification_data();
  void message_notification();
  void message_load_state_data();
  void message_load_state();
  void message_dismiss();
};
