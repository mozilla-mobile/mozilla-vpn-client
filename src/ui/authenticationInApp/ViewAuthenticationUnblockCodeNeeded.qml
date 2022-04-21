/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1


VPNInAppAuthenticationBase {
    // TODO
    // If we are here, we were trying to complete the authentication flow using
    // an existing account. That account has not been verified yet. The user
    // needs to insert the 6-digit code. The code expires after 5 minutes. Use
    // `resendUnblockCodeEmail` if needed.
    // After this step, call `verifyUnblockCode()` with the code. If the code is
    // not valid, an error will be signaled.
    // The next steps are:
    // - Sign-in again.
    // - errors.

    id: authSignUp

    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => { VPNAuthInApp.reset() }
    _menuButtonAccessibleName: qsTrId("vpn.connectionInfo.close")
    _headlineText: VPNl18n.InAppAuthVerificationCodeTitle
    _subtitleText: VPNl18n.InAppAuthEmailVerificationDescription
    _imgSource: "qrc:/nebula/resources/verification-code.svg"

    _inputs: VPNInAppAuthenticationInputs {
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateUnblockCodeNeeded && activeInput().text.length === VPNAuthInApp.unblockCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { VPNAuthInApp.verifyUnblockCode(inputText) }
        _buttonText: VPNl18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhNone
        _inputPlaceholderText: VPNl18n.InAppAuthUnblockCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: VPNl18n.InAppAuthResendCodeLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPNAuthInApp.resendUnblockCodeEmail();
        }
        VPNCancelButton {
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPN.cancelAuthentication()
        }
    }
}
