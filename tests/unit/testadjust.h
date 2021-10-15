/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestAdjust final : public TestHelper {
  Q_OBJECT

 private slots:
  void paramFiltering_data();
  void paramFiltering();

  void stateMachine_data();
  void stateMachine();
};
