/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

Item {
  // TODO
  // This view should inform the user that "something is happening".
  // In the background, we are starting the authentication flow.
  // The next states are:
  // - StateStart: happy path.
  // - Authentication failed, network down, etc.

  Component.onCompleted: console.log("INITIALIZE")
}
