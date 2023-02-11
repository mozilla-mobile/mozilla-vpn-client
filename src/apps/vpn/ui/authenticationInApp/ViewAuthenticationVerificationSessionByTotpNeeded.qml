/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

VPNInAppAuthenticationBase {
    _viewObjectName: "authVerificationSessionByTotpNeeded"
    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            cancelAuthenticationFlow();
        } else {
            VPN.cancelAuthentication();
        }
    }
    _menuButtonAccessibleName: qsTrId("vpn.connectionInfo.close")
    _headlineText: VPNI18n.InAppAuthSecurityCodeTitle
    _subtitleText: VPNI18n.InAppAuthSecurityCodeSubtitle
    _imgSource: "qrc:/nebula/resources/verification-code.svg"
    _inputLabel: VPNI18n.InAppAuthSecurityCodeLabel

    _inputs: VPNInAppAuthenticationInputs {
        objectName: "authVerificationSessionByTotpNeeded"

        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByTotpNeeded && activeInput().text.length === VPNAuthInApp.totpCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { VPNAuthInApp.verifySessionTotpCode(inputText) }
        _buttonText: VPNI18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhDigitsOnly
        _inputPlaceholderText: VPNI18n.InAppAuthSecurityCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter

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
