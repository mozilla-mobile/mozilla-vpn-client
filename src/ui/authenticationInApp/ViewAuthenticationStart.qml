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
    // The next step is to call VPNAuthInApp::sigInOrUp();

    Component.onCompleted: console.log("START")

    VPNTextField {
        id: emailInput

        anchors.top: parent.top
        anchors.bottomMargin: 24
        width: parent.width

        placeholderText: "a@b.c" // TODO
    }

    VPNTextField {
        id: passwordInput

        anchors.top: emailInput.bottom
        anchors.bottomMargin: 24
        width: parent.width
        echoMode: TextInput.Password

        placeholderText: "secure password" // TODO
    }

    VPNButton {
        anchors.top: passwordInput.bottom
        anchors.bottomMargin: 24
        text: "SignIn" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPNAuthInApp.signInOrUp(emailInput.text, passwordInput.text); // TODO: email check?
    }
}
