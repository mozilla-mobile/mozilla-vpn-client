/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestIllegalNoop final : public TestHelper {
  Q_OBJECT

 private slots:
  void setup();

  // No operations here. We test that the app fails to start with no params
  void no_params();
  // Make sure if the params are wrong, the app fails to start
  void wrong_param();
  // Make sure if the first 2 are correct, but there are too many, the app
  // fails.
  void partially_wrong_param();
};
