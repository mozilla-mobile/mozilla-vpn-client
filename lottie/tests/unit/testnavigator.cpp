/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testnavigator.h"

#include <QJSEngine>

#include "../../lib/lottieprivate.h"
#include "../../lib/lottieprivatenavigator.h"

void TestNavigator::userAgent() {
  QJSEngine engine;
  LottiePrivate::initialize(&engine, "Foo 1.0");

  LottiePrivate p;
  LottiePrivateNavigator navigator(&p);

  QCOMPARE(navigator.userAgent(), "Foo 1.0");
}

static TestNavigator s_testNavigator;
