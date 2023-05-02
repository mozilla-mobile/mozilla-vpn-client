/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "teststatusicon.h"

#include <QEventLoop>

#include "statusicon.h"

void TestStatusIcon::basic() {
  StatusIcon si;
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask.png");

  si.refreshNeeded();
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask.png");

  // VPN is on
  TestHelper::vpnState = MozillaVPN::StateMain;
  TestHelper::controllerState = Controller::StateOn;
  si.refreshNeeded();

  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask-on.png");

  // VPN is off
  TestHelper::controllerState = Controller::StateOff;
  si.refreshNeeded();
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask-off.png");

  // VPN is switching
  TestHelper::controllerState = Controller::StateSwitching;

  int i = 0;
  QEventLoop loop;
  connect(&si, &StatusIcon::iconUpdateNeeded, &si, [&]() {
    if (i > 10) {
      si.disconnect();
      loop.exit();
      return;
    }

    QCOMPARE(
        si.iconString(),
        QString(":/ui/resources/logo-animated-mask%1.png").arg((i % 4) + 1));
    ++i;
  });

  si.refreshNeeded();
  QCOMPARE(si.iconString(), ":/ui/resources/logo-animated-mask1.png");
  loop.exec();
}

static TestStatusIcon s_testStatusIcon;
