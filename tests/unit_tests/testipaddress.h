/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestIpAddress final : public TestHelper {
  Q_OBJECT

 private slots:
  void ctor();

  void basic_data();
  void basic();

  void overlaps_data();
  void overlaps();

  void contains_data();
  void contains();

  void equal_data();
  void equal();

  void excludeAddresses_data();
  void excludeAddresses();
};
