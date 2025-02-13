/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestDaemonAccessControl final : public TestHelper {
  Q_OBJECT

 private slots:
  void initTestCase();
  /**
   * * "status" command is allowed for any peer when inactive
   * * "activate" command is allowed for any peer when inactive
   * * commands other than "activate" or "status" are not allowed
   *
   */
  void testCommandsWhenInactive();

  /**
   * root i.e. peerId = 0 is always authorized regardles of session
   * owner.
   *
   */
  void testCommandsWhenPeerIsRoot();
  /**
   * Other than root, only the session owner may change the current
   * session.
   *
   */
  void testAuthorizedPeerWhenActive();
  void testUnauthorizedPeerWhenActive();

  /**
   * When a session is reset the access control returns to the inactive
   * state.
   *
   */
  void testSessionReset();
};
