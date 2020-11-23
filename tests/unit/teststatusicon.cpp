/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "teststatusicon.h"
#include "../../src/statusicon.h"

void TestStatusIcon::basic() {
  StatusIcon si;
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-generic.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.svg");

  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-generic.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.svg");

  TestHelper::vpnState = MozillaVPN::StateMain;
  TestHelper::controllerState = Controller::StateOn;
  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-on.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-on.svg");

  TestHelper::controllerState = Controller::StateOff;
  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-generic.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.svg");

  TestHelper::controllerState = Controller::StateSwitching;
  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-animated1.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-animated1.svg");

  TestHelper::controllerState = Controller::StateCaptivePortal;
  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-generic.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.svg");
}

static TestStatusIcon s_testStatusIcon;
