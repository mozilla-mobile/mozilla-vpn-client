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
    _viewObjectName: "authUnblockCodeNeeded"
    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            cancelAuthenticationFlow();
        } else {
            VPNAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: qsTrId("vpn.connectionInfo.close")
    _headlineText: VPNI18n.InAppAuthVerificationCodeTitle
    _subtitleText: VPNI18n.InAppAuthEmailVerificationDescription
    _imgSource: "qrc:/nebula/resources/verification-code.svg"

    _inputs: VPNInAppAuthenticationInputs {
        objectName: "authUnblockCodeNeeded"
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateUnblockCodeNeeded && activeInput().text.length === VPNAuthInApp.unblockCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { VPNAuthInApp.verifyUnblockCode(inputText) }
        _buttonText: VPNI18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhNone
        _inputPlaceholderText: VPNI18n.InAppAuthUnblockCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: VPNI18n.InAppAuthResendCodeLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                VPNAuthInApp.resendUnblockCodeEmail();
                VPNErrorHandler.requestAlert(VPNErrorHandler.AuthCodeSentAlert);
            
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
