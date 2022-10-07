/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1


VPNInAppAuthenticationBase {
    id: authSignUp

    _viewObjectName: "authVerificationSessionByEmailNeeded"
    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            cancelAuthenticationFlow();
        } else {
            VPNAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: qsTrId("vpn.connectionInfo.close")
    _headlineText: VPNl18n.InAppAuthVerificationCodeTitle
    _subtitleText: VPNl18n.InAppAuthEmailVerificationDescription
    _imgSource: "qrc:/nebula/resources/verification-code.svg"

    _inputs: VPNInAppAuthenticationInputs {
        objectName: "authVerificationSessionByEmailNeeded"
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
            onClicked: {
                VPNAuthInApp.resendVerificationSessionCodeEmail();
                VPNErrorHandler.setAlert(VPNErrorHandler.AuthCodeSentAlert);
            }
        }

        VPNCancelButton {
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                if (isReauthFlow) {
                    cancelAuthenticationFlow();
                } else {
                    VPN.cancelAuthentication();
                }
            }
        }

    }

}
