/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

#include "testhelper.h"

class TestServerModels final : public QObject, TestHelper<TestServerModels> {
  Q_OBJECT

 private slots:
  void serverBasic();
  void serverFromJson_data();
  void serverFromJson();
  void serverWeightChooser();

  void serverCityBasic();
  void serverCityFromJson_data();
  void serverCityFromJson();

  void serverCountryBasic();
  void serverCountryFromJson_data();
  void serverCountryFromJson();

  void serverCountryModelBasic();
  void serverCountryModelFromJson_data();
  void serverCountryModelFromJson();
};
