/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZInAppAuthenticationBase {
    id: authUnblockCodeNeeded
    objectName: "authUnblockCodeNeeded"

    _menuButtonImageSource: MZAssetLookup.getImageSource("CloseDark")
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            cancelAuthenticationFlow();
        } else {
            MZAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: MZI18n.GlobalClose
    _headlineText: MZI18n.InAppAuthVerificationCodeTitle
    _subtitleText: MZI18n.InAppAuthEmailVerificationDescription
    _imgSource: MZAssetLookup.getImageSource("VerificationCode")

    _inputs: MZInAppAuthenticationInputs {
        _viewObjectName: authUnblockCodeNeeded.objectName
        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateUnblockCodeNeeded && activeInput().text.length === MZAuthInApp.unblockCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { MZAuthInApp.verifyUnblockCode(inputText) }
        _buttonText: MZI18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhNone
        _inputPlaceholderText: MZI18n.InAppAuthUnblockCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: MZTheme.theme.windowMargin

        MZLinkButton {
            objectName: authUnblockCodeNeeded.objectName + "-resendCode"
            labelText: MZI18n.InAppAuthResendCodeLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                MZAuthInApp.resendUnblockCodeEmail();
                MZErrorHandler.requestAlert(MZErrorHandler.AuthCodeSentAlert);

            }
        }
        MZCancelButton {
            objectName: authUnblockCodeNeeded.objectName + "-cancel"
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
