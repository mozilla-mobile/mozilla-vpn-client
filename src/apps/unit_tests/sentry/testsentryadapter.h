/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestSentryAdapter final : public TestHelper {
  Q_OBJECT

 private slots:
  void init_creates_task();
  void init_no_task_when_init();

};
