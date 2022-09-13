/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "teststatusicon.h"
#include "../../src/statusicon.h"

#include <QEventLoop>

void TestStatusIcon::basic() {
  StatusIcon si;
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask.png");
  QCOMPARE(si.indicatorColor().isValid(), false);

  si.refreshNeeded();
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask.png");
  QCOMPARE(si.indicatorColor().isValid(), false);

  // VPN is on
  TestHelper::vpnState = MozillaVPN::StateMain;
  TestHelper::controllerState = Controller::StateOn;
  si.refreshNeeded();

  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask-on.png");
  QCOMPARE(si.indicatorColor().isValid(), true);

  // VPN is off
  TestHelper::controllerState = Controller::StateOff;
  si.refreshNeeded();

  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask-off.png");
  QCOMPARE(si.indicatorColor().isValid(), false);

  // VPN is switching
  TestHelper::controllerState = Controller::StateSwitching;

  int i = 0;
  QEventLoop loop;
  connect(&si, &StatusIcon::iconUpdateNeeded, [&]() {
    if (i > 10) {
      si.disconnect();
      loop.exit();
      return;
    }

    QCOMPARE(si.iconString(),
             QString(":/ui/resources/logo-animated%1.png").arg((i % 4) + 1));
    ++i;
  });

  si.refreshNeeded();
  QCOMPARE(si.iconString(), ":/ui/resources/logo-animated1.png");
  QCOMPARE(si.indicatorColor().isValid(), false);
  loop.exec();
}

static TestStatusIcon s_testStatusIcon;
