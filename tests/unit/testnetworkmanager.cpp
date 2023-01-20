/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testnetworkmanager.h"

#include "helper.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

void TestNetworkManager::basic() {
  SettingsHolder settingsHolder;
  SimpleNetworkManager snm;
  QCOMPARE(&snm, NetworkManager::instance());
  QVERIFY(snm.userAgent().contains("MozillaVPN"));
  QCOMPARE(snm.networkAccessManager(), snm.networkAccessManager());
}

static TestNetworkManager s_testNetworkManager;
