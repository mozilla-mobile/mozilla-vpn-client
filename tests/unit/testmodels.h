/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestModels final : public TestHelper {
  Q_OBJECT

 private slots:
  void cleanup();
  void apiErrorBasic();
  void apiErrorParse();
  void apiErrorParse_data();
  void apiErrorInvalid();
  void apiErrorInvalid_data();

  void deviceBasic();
  void deviceCurrentDeviceName();
  void deviceFromJson_data();
  void deviceFromJson();

  void deviceModelBasic();
  void deviceModelFromJson_data();
  void deviceModelFromJson();
  void deviceModelRemoval();

  void keysBasic();

  void recentConnectionBasic();
  void recentConnectionMigration_data();
  void recentConnectionMigration();
  void recentConnectionSaveAndRestore();

  void recommendedLocationsPick();

  void serverDataBasic();
  void serverDataMigrate();

  void userBasic();
  void userFromJson_data();
  void userFromJson();
  void userFromSettings();

  void locationBasic();
  void locationFromJson_data();
  void locationFromJson();
  void locationDistance_data();
  void locationDistance();
};
