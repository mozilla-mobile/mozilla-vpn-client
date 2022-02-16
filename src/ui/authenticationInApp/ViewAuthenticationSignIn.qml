/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1

VPNInAppAuthenticationBase {

    // TODO
    // We are completing an authentication using an existing account.
    // There is nothing to do here, except... waiting.
    // There 5 possible next-steps:
    // - authentication completed (VPN.state will change, the authentication is
    //   completed)
    // - unblock code needed. This can happen for security reasons. We go
    //   to UnblockCodeNeeded. The user needs to insert the 6-digit code.
    // - The user enters the wrong password, sees error.
    // - The user clicks "Change email", "Cancel", or the back arrow and goes back to start.
    // - Some other error, goes back to start and sees error.

    // TODOs (likely there are more)
    // Add final content
    // Open forgot password flow in webview on click
    // Add TOS and privacy links. Can borrow from ViewSubscriptionNeeded.qml.
    // Show form error messages
    // Form interaction polish
    // Add password criteria tooltip

    _changeEmailLinkVisible: true
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => { VPNAuthInApp.reset() }
    _menuButtonAccessibleName: "Back"
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor"
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: "Enter password"

    _inputs: ColumnLayout {
        spacing: VPNTheme.theme.vSpacing * 2
        VPNPasswordInput {
            id: passwordInput
            Layout.fillWidth: true
            _placeholderText: "secure password" // TODO
        }

        VPNButton {
            text: "Sign in"
            Layout.fillWidth: true
            onClicked: {
                VPNAuthInApp.setPassword(passwordInput.text);
                VPNAuthInApp.signIn();
            }
        }
    }

    _disclaimers: RowLayout {
        Layout.alignment: Qt.AlignHCenter
        VPNTextBlock {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
        }
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: "Forgot your password?"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        VPNLinkButton {
            labelText: "Cancel"
            fontName: VPNTheme.theme.fontBoldFamily
            anchors.horizontalCenter: parent.horizontalCenter
            linkColor: VPNTheme.theme.redButton
            onClicked: VPNAuthInApp.reset()
        }

    }
}
