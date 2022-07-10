/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestAddon final : public TestHelper {
  Q_OBJECT

 private slots:
  void conditions_data();
  void conditions();

  void conditionWatcher_locale();
  void conditionWatcher_group();
  void conditionWatcher_triggerTime();

  void guide_create_data();
  void guide_create();

  void tutorial_create_data();
  void tutorial_create();

  void message_create_data();
  void message_create();

  void message_dismiss();
};
