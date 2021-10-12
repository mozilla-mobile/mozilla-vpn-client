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
    // The TOTP code is required.
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

    Component.onCompleted: console.log("SESSION VERIFICATION BY TOTP")

    Text {
        id: msg
        text: "TOTP verification needed. Code:"
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
        onClicked: VPNAuthInApp.verifySessionTotpCode(codeInput.text);
    }
}
