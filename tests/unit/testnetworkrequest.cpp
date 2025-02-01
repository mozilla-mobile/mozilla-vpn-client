/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testnetworkrequest.h"

#include "networkrequest.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"
#include "tasks/function/taskfunction.h"

void TestNetworkRequest::testSetAuthHeader() {
  SettingsHolder settingsHolder;
  SimpleNetworkManager snm;

  TaskFunction task([&]() {});
  NetworkRequest request(&task);

  // If this is not set we will hit a Q_ASSERT.
  settingsHolder.setToken("TOKEN");

  // Test that if no token is provided we use what is the settings holder.
  request.auth();
  QCOMPARE(request.m_request.rawHeader("Authorization"), "Bearer TOKEN");

  // Test that if a token is provided we use that.
  request.auth("ANOTHER TOKEN");
  QCOMPARE(request.m_request.rawHeader("Authorization"), "ANOTHER TOKEN");
}

static TestNetworkRequest s_testNetworkRequest;
