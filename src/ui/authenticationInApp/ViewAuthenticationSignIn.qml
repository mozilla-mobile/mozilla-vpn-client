/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

Item {
    // TODO
    // We are completing an authentication using an existing account.
    // There is nothing to do here, except... waiting.
    // There 3 possible next-steps:
    // - authentication completed (VPN.state will change, the authentication is
    //   completed)
    // - email verification needed. This can happen for security reasons. We go
    //   to EmailVerification. The user needs to insert the 6-digit code.
    //   Then we go back to SignIn state.
    // - errors... for instance: the password is wrong. See the ErrorType enum.

    Component.onCompleted: console.log("SIGN IN")
}
