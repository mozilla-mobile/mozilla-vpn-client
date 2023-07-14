/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestQmlPath : public TestHelper {
  Q_OBJECT

 private slots:
  void parse_data();
  void parse();

  void evaluate_data();
  void evaluate();
};
