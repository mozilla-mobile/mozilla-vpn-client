/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"

Item {
    // TODO
    // This view shows the authentication form. We need to collect an email
    // address and a password.
    // At this point we do not know if we are going to sign-in or sign-up.

    Component.onCompleted: console.log("START")

    VPNTextField {
        id: emailInput

        anchors.top: parent.top
        anchors.bottomMargin: 24
        width: parent.width
        hasError: !VPNAuthInApp.validateEmailAddress(emailInput.text)

        placeholderText: "a@b.c" // TODO
    }

    VPNButton {
        anchors.top: emailInput.bottom
        anchors.bottomMargin: 24
        text: "Continue" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPNAuthInApp.checkAccount(emailInput.text); // TODO: email check?
    }
}
