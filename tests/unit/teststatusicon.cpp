/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "teststatusicon.h"
#include "../../src/statusicon.h"

#include <QEventLoop>

void TestStatusIcon::basic() {
  StatusIcon si;
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.png");
#else
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask.png");
#endif

  si.stateChanged();
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.png");
#else
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask.png");
#endif

  TestHelper::vpnState = MozillaVPN::StateMain;
  TestHelper::controllerState = Controller::StateOn;
  si.stateChanged();
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
  QCOMPARE(si.iconString(), ":/ui/resources/logo-on.png");
#else
           "qrc:/ui/resources/logo-generic-mask-on.png");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask-on.png");
#endif

  TestHelper::controllerState = Controller::StateOff;
  si.stateChanged();
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.png");
#else
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic-mask-off.png");
#endif

  TestHelper::controllerState = Controller::StateSwitching;

  int i = 0;
  QEventLoop loop;
  connect(&si, &StatusIcon::iconUpdateNeeded, [&]() {
    if (i > 10) {
      si.disconnect();
      loop.exit();
      return;
    }
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
    QCOMPARE(si.iconString(),
             QString(":/ui/resources/logo-animated%1.png").arg((i % 4) + 1));
#else
    QCOMPARE(si.iconString(),
             QString(":/ui/resources/logo-animated-mask%1.png")
             .arg((i % 4) + 1));
#endif
    ++i;
  });

  si.stateChanged();
#if defined(MVPN_LINUX) || defined(MVPN_WINDOWS)
  QCOMPARE(si.iconString(), ":/ui/resources/logo-animated1.png");
#else
  QCOMPARE(si.iconString(), ":/ui/resources/logo-animated-mask1.png");
#endif
  loop.exec();
}

static TestStatusIcon s_testStatusIcon;
