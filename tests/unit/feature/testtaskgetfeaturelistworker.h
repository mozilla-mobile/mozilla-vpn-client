/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestTaskGetFeatureListWorker final : public TestHelper {
  Q_OBJECT

 private slots:
  void testTaskIsScheduledOnStart();
  void testTaskIsScheduledPeriodically();
  void testTaskIsScheduledOnTokenChange();
  void testTimerIsStoppedOnDestruction();
};
