/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QObject>

#include "testdaemon.h"

class TestSwitch : public TestDaemon {
  Q_OBJECT

 private slots:
  void init();
  void basicSwitch();
  void hardSwitch();

 private:
  InterfaceConfig m_config;
};
