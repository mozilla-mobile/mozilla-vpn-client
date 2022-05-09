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

    id: authSignUp

    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => {
        if (isDeleteAccountAuth) {
            cancelAccountDeletion();
        } else {
            VPNAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: qsTrId("vpn.connectionInfo.close")
    _headlineText: VPNl18n.InAppAuthVerificationCodeTitle
    _subtitleText: VPNl18n.InAppAuthEmailVerificationDescription
    _imgSource: "qrc:/nebula/resources/verification-code.svg"

    _inputs: VPNInAppAuthenticationInputs {
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByEmailNeeded && activeInput().text && activeInput().text.length === VPNAuthInApp.sessionEmailCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { VPNAuthInApp.verifySessionEmailCode(inputText) }
        _buttonText: VPNl18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhNone
        _inputPlaceholderText: VPNl18n.InAppAuthSessionEmailCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: VPNl18n.InAppAuthResendCodeLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPNAuthInApp.resendVerificationSessionCodeEmail();
        }

        VPNCancelButton {
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                if (isDeleteAccountAuth) {
                    cancelAccountDeletion();
                } else {
                    VPN.cancelAuthentication();
                }
            }
        }

    }

}
