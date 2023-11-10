/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdaemonaccesscontrol.h"

#include "daemon/daemonaccesscontrol.h"

void TestDaemonAccessControl::initTestCase() {
  // Reset the session before each test case.
  DaemonAccessControl::instance()->resetSession();
}

void TestDaemonAccessControl::testInactiveCommands() {
  auto dac = DaemonAccessControl::instance();

  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("deactivate", nullptr));

  // A peer id of -1 means activation failed,
  // access crontrol defaults to unauthorized.
  dac->setPeerId(-1);
  QVERIFY(!dac->isCommandAuthorizedForPeer("activate", nullptr));
  // Set peer id to something valid for activateion to succed.
  dac->setPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
}

void TestDaemonAccessControl::testRootOpenAccess() {
  auto dac = DaemonAccessControl::instance();

  // Set the peer id to a normal user peer id, not root and activate.
  // That will make `1` the session owner.
  dac->setPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  // Now set the peer id to root, root should be allowed to do anything.
  dac->setPeerId(0);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("deactivate", nullptr));
}

void TestDaemonAccessControl::testActiveAuthorizedPeer() {
  auto dac = DaemonAccessControl::instance();

  // Set the peer id to a normal user peer id and activate.
  // That will make `1` the session owner.
  dac->setPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  // Without changing the peer id, check that all commands are authorized.
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("deactivate", nullptr));
}

void TestDaemonAccessControl::testActiveUnauthorizedPeer() {
  auto dac = DaemonAccessControl::instance();

  // Set the peer id to a normal user peer id and activate.
  // That will make `1` the session owner.
  dac->setPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  // Change the peer id to another non-root user,
  // check that all commands are NOT authorized.
  dac->setPeerId(2);
  QVERIFY(!dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("deactivate", nullptr));
}

void TestDaemonAccessControl::testSessionReset() {
  auto dac = DaemonAccessControl::instance();

  // Set the peer id to a normal user peer id and activate.
  // That will make `1` the session owner.
  dac->setPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  dac->resetSession();

  // Now reset the session and check that the new user is authorized, not `1`.
  dac->setPeerId(2);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("deactivate", nullptr));

  // Overzealously check that `1` is not authorized.
  dac->setPeerId(1);
  QVERIFY(!dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("deactivate", nullptr));
}

static TestDaemonAccessControl s_testDaemonAccessControl;
