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

    _changeEmailLinkVisible: true
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        VPNAuthInApp.reset();
    }
    _menuButtonAccessibleName: "Back"
    _headlineText: "Enter verification code"
    _subtitleText: "Open your email and enter the verification code it that was sent."
    _imgSource: "qrc:/nebula/resources/verification-code.svg"

    _inputs: ColumnLayout {
        spacing: VPNTheme.theme.vSpacingSmall

        VPNBoldLabel {
            id: codeLabel
            text: "Verification code"
        }

        VPNTextField {
            id: codeInput
            hasError: false
            _placeholderText: "Enter 6-digit code"
            Layout.fillWidth: true
        }

        VPNContextualAlerts {
            id: passwordInputCreateWarnings
            Layout.fillWidth: true

            messages: [
                {
                    type: "error",
                    message: "Invalid code entry",
                    visible: !createAccountButton.enabled
                }
            ]
        }

        VPNButton {
            id: createAccountButton
            enabled: codeInput.text && codeInput.text.length === VPNAuthInApp.getVerificationCodeLength()
            text: "Verify" + VPNAuthInApp.getVerificationCodeLength()
            Layout.fillWidth: true

            onClicked: {
                if (enabled) {
                    VPNAuthInApp.verifySessionEmailCode(codeInput.text);
                }
            }
        }
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: "Resend code"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                VPNAuthInApp.resendVerificationSessionCodeEmail();
            }
        }

        VPNLinkButton {
            labelText: "Cancel"
            fontName: VPNTheme.theme.fontBoldFamily
            anchors.horizontalCenter: parent.horizontalCenter
            linkColor: VPNTheme.theme.redButton
            onClicked: VPNAuthInApp.reset()
        }

    }

    Component.onCompleted: {
        console.log("SESSION VERIFICATION BY EMAIL");
    }

}
