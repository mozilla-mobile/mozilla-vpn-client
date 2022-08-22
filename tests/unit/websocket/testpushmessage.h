/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class TestPushMessage : public TestHelper {
  Q_OBJECT

 private slots:
  void tst_invalidMessagesAreIgnored();
  void tst_validMessagesAreParsedAndExecuted();

  // Message handler tests are under
  // `test/functional/testPushMessageHandlers.js`, because these rely too much
  // on the MozillaVPN singleton state and so cannot be easily tests as unit
  // tests where that singletong is mostly no-op.
};
