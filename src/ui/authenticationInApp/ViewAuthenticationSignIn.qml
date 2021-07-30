/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme
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

    Component.onCompleted: {
        console.log("SIGN IN");
        passwordInput.forceActiveFocus();

    }

    ColumnLayout {
        id: col
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -Theme.vSpacing
        spacing: Theme.windowMargin

        VPNHeadline {
            text: "Welcome back,<br/> {{ email address }}"
            Layout.bottomMargin: Theme.vSpacing
            Layout.preferredWidth: parent.width
        }

        VPNTextField {
            id: passwordInput

            Layout.fillWidth: true
            echoMode: TextInput.Password
            placeholderText: "Enter secure password" // TODO

            Keys.onReturnPressed: {
                if (passwordInput.length === 0) {
                    return;
                }
                btn.clicked();
            }
        }

        VPNButton {
            id: btn
            Layout.fillWidth: true
            text: "Sign In" // TODO
            onClicked: {

                passwordInput.enabled = false;
                col.opacity = .5;
                VPNAuthInApp.setPassword(passwordInput.text);
                VPNAuthInApp.signIn();
            }
        }

        VPNVerticalSpacer {
            height: 8
        }

    }
    VPNLinkButton {
        labelText: "Start over with another email"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.windowMargin / 2
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: console.log("TODO: we need to bail out and go back to email address entry")
        fontSize: Theme.fontSizeSmall
    }

}
