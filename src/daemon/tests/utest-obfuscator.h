/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

#include "testdaemon.h"

class TestObfuscator : public TestDaemon {
  Q_OBJECT

 private slots:
  void activate();
  void switchServerAndMethod();
  void switchServerAddObfuscator();
  void switchServerAndFailure();

  void startupFailure();
  void crash();
};
