/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

Item {
    // TODO
    // If we are here, we were trying to complete the authentication flow using
    // an existing account. That account has not been verified yet. The user
    // needs to insert the 6-digit code.
    // After this step, call `verifyEmailCode()` with the code. If the code is
    // not valid, an error will be signaled.
    // The next steps are:
    // - Sign-in again.
    // - errors.
    // TODO: resend the verify-email-code. (please keep this TODO!)

    Component.onCompleted: console.log("EMAIL VERIFICATION")

    Text {
        id: msg
        text: "Email verification needed. Code:"
        anchors.top: parent.top
    }

    VPNTextField {
        id: codeInput

        anchors.top: msg.bottom
        anchors.bottomMargin: 24
        width: parent.width
    }

    VPNButton {
        anchors.top: codeInput.bottom
        anchors.bottomMargin: 24
        text: "Verify" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPNAuthInApp.verifyEmailCode(codeInput.text);
    }
}
