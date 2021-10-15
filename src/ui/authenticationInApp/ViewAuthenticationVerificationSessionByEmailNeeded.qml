/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

Item {
    // TODO
    // we are here if the user is completing the session activation flow.
    // FxA has sent an email with a 6-digit code. The code must be received by
    // this view and then passed to the C++ layer using:
    // `verifySessionEmailCode(code)`.
    // The code expires after 5 minutes. If the user needs a new code, call:
    // `resendVerificationSessionCodeEmail()`.
    //
    // After this step we can go to:
    // - all done \o/
    // - error
    //
    // Note that "all done" means IAP! So, more likely, you will receive
    // errors, crashes, a lot of fun things... nothing will work, and more
    // likely the app will implode.
    // In theory, this part should be tested on iOS or on Android when we will
    // have IAP there too.

    Component.onCompleted: console.log("SESSION VERIFICATION BY EMAIL")

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
        id: codeButton
        anchors.top: codeInput.bottom
        anchors.bottomMargin: 24
        text: "Verify" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPNAuthInApp.verifySessionEmailCode(codeInput.text);
    }

    VPNButton {
        anchors.top: codeButton.bottom
        anchors.bottomMargin: 24
        text: "Send new code" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPNAuthInApp.resendVerificationSessionCodeEmail();
    }
}
