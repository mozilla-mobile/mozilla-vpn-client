/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "teststatusicon.h"
#include "../../src/statusicon.h"

#include <QEventLoop>

void TestStatusIcon::basic() {
  StatusIcon si;
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-generic.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.svg");

  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-generic.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.svg");

  TestHelper::coreState = Core::StateMain;
  TestHelper::controllerState = Controller::StateOn;
  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-on.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-on.svg");

  TestHelper::controllerState = Controller::StateOff;
  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-generic.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-generic.svg");

  TestHelper::controllerState = Controller::StateSwitching;

  int i = 0;
  QEventLoop loop;
  connect(&si, &StatusIcon::iconChanged, [&]() {
    if (i > 10) {
      si.disconnect();
      loop.exit();
      return;
    }

    QCOMPARE(si.iconUrl().toString(),
             QString("qrc:/ui/resources/logo-animated%1.svg").arg((i % 4) + 1));
    QCOMPARE(si.iconString(),
             QString(":/ui/resources/logo-animated%1.svg").arg((i % 4) + 1));
    ++i;
  });

  si.stateChanged();
  QCOMPARE(si.iconUrl().toString(), "qrc:/ui/resources/logo-animated1.svg");
  QCOMPARE(si.iconString(), ":/ui/resources/logo-animated1.svg");
  loop.exec();
}

static TestStatusIcon s_testStatusIcon;
