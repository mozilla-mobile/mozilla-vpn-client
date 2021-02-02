/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestAndroidMigration final : public TestHelper {
  Q_OBJECT

 private slots:
  void deviceInfo_data();
  void deviceInfo();

  void userInfo_data();
  void userInfo();

  void serverList_data();
  void serverList();
};
