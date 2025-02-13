/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdaemonaccesscontrol.h"

#include "daemon/daemonaccesscontrol.h"

void TestDaemonAccessControl::initTestCase() {
  // Reset the session before each test case.
  DaemonAccessControl::instance()->resetSession();
}

void TestDaemonAccessControl::testCommandsWhenInactive() {
  auto dac = DaemonAccessControl::instance();

  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("deactivate", nullptr));

  // A peer id of -1 means activation failed,
  // access control defaults to unauthorized.
  dac->setMockPeerId(-1);
  QVERIFY(!dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  // Set peer id to something valid for activation to succeed.
  dac->setMockPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
}

void TestDaemonAccessControl::testCommandsWhenPeerIsRoot() {
  auto dac = DaemonAccessControl::instance();

  // Set the peer id to a normal user peer id, not root and activate.
  // That will make `1` the session owner.
  dac->setMockPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  // Now set the peer id to root, root should be allowed to do anything.
  dac->setMockPeerId(0);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("deactivate", nullptr));
}

void TestDaemonAccessControl::testAuthorizedPeerWhenActive() {
  auto dac = DaemonAccessControl::instance();

  // Set the peer id to a normal user peer id and activate.
  // That will make `1` the session owner.
  dac->setMockPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  // Without changing the peer id, check that all commands are authorized.
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("deactivate", nullptr));
}

void TestDaemonAccessControl::testUnauthorizedPeerWhenActive() {
  auto dac = DaemonAccessControl::instance();

  // Set the peer id to a normal user peer id and activate.
  // That will make `1` the session owner.
  dac->setMockPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  // Change the peer id to another non-root user,
  // check that all commands are NOT authorized.
  dac->setMockPeerId(2);
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
  dac->setMockPeerId(1);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));

  dac->resetSession();

  // Now reset the session and check that the new user is authorized, not `1`.
  dac->setMockPeerId(2);
  QVERIFY(dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(dac->isCommandAuthorizedForPeer("deactivate", nullptr));

  // Overzealously check that `1` is not authorized.
  dac->setMockPeerId(1);
  QVERIFY(!dac->isCommandAuthorizedForPeer("activate", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("status", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("logs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("cleanlogs", nullptr));
  QVERIFY(!dac->isCommandAuthorizedForPeer("deactivate", nullptr));
}

static TestDaemonAccessControl s_testDaemonAccessControl;
