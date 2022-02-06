/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

Item {
    // TODO
    // We are completing an authentication using an existing account.
    // There is nothing to do here, except... waiting.
    // There 3 possible next-steps:
    // - authentication completed (VPN.state will change, the authentication is
    //   completed)
    // - unblock code needed. This can happen for security reasons. We go
    //   to UnblockCodeNeeded. The user needs to insert the 6-digit code.
    //   Then we go back to SignIn state.
    // - errors... for instance: the password is wrong. See the ErrorType enum.

    Component.onCompleted: console.log("SIGN IN")

    VPNTextField {
        id: passwordInput

        anchors.top: parent.top
        anchors.bottomMargin: 24
        width: parent.width

        echoMode: TextInput.Password

        _placeholderText: "secure password" // TODO
    }

    VPNButton {
        id: signInButton

        anchors.top: passwordInput.bottom
        anchors.bottomMargin: 24
        text: "Sign In" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: {
          VPNAuthInApp.setPassword(passwordInput.text);
          VPNAuthInApp.signIn();
        }
    }

    VPNButton {
        anchors.top: signInButton.bottom
        anchors.bottomMargin: 24
        text: "Reset" // TODO
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPNAuthInApp.reset();
    }
}
