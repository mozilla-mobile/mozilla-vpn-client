/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
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
    // - The user clicks "Change email" or the back arrow and goes back to start.
    // - Some other error, goes back to start and sees error.

    // TODOs (likely there are more)
    // Open forgot password flow in webview on click
    // Form interaction polish
    // Add password criteria tooltip
    // Maybe add button loader

    _changeEmailLinkVisible: true
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => { VPNAuthInApp.reset() }
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: VPNl18n.InAppAuthSignInSubtitle
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: VPNl18n.InAppAuthPasswordInputLabel

    _inputs: VPNInAppAuthenticationInputs {
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateSignIn && !activeInput().hasError
        _buttonOnClicked: (inputText) => {
             VPNAuthInApp.setPassword(inputText);
             VPNAuthInApp.signIn();
         }
        _buttonText: VPNl18n.InAppAuthSignInButton
        _inputPlaceholderText: VPNl18n.InAppAuthPasswordInputPlaceholder
    }

    _disclaimers: VPNInAppAuthenticationLegalDisclaimer {}

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: VPNl18n.InAppAuthForgotPasswordLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPN.openLink(VPN.LinkForgotPassword)
        }

        VPNInAppAuthenticationCancel {}
    }
}
