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
    // We show this view when the user is creating an account.
    // Things are happening in background. The next steps are:
    // - Verification by email needed
    // - errors

    Component.onCompleted: console.log("SIGN UP")


    ColumnLayout {
        id: col
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        spacing: Theme.windowMargin

        VPNHeadline {
            text: "Create account"
            Layout.preferredWidth: parent.width
        }

        VPNTextBlock {
            text: "Enter password to create a new account with newUserEmail@somenewEmail.com"
            width: undefined
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.pixelSize: 12
            Layout.leftMargin: Theme.windowMargin
            Layout.rightMargin: Theme.windowMargin
        }

        ColumnLayout {
            spacing: 8
            Layout.fillWidth: true

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

            VPNTextField {
                id: verifyPassword

                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: "Verify password" // TODO

                Keys.onReturnPressed: {
                    if (passwordInput.length === 0) {
                        return;
                    }
                    btn.clicked();
                }
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
