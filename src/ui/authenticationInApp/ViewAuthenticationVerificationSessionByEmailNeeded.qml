/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

MZInAppAuthenticationBase {
    id: authSignUp
    objectName: "authVerificationSessionByEmailNeeded"

    _telemetryScreenId: "enter_verification_code"
    _headlineText: MZI18n.InAppAuthVerificationCodeTitle
    _subtitleText: MZI18n.InAppAuthEmailVerificationDescription
    _imgSource: "qrc:/nebula/resources/verification-code.svg"
    _backButtonVisible: false

    _inputs: MZInAppAuthenticationInputs {
        _viewObjectName: authSignUp.objectName
        _telemetryScreenId: authSignUp._telemetryScreenId
        _telemetryButtonEventName: "verifySelected"
        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByEmailNeeded && activeInput().text && activeInput().text.length === MZAuthInApp.sessionEmailCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { MZAuthInApp.verifySessionEmailCode(inputText) }
        _buttonText: MZI18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhNone
        _inputPlaceholderText: MZI18n.InAppAuthSessionEmailCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: MZTheme.theme.windowMargin

        MZLinkButton {
            objectName: authSignUp.objectName + "-resendCode"
            labelText: MZI18n.InAppAuthResendCodeLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                Glean.interaction.resendCodeSelected.record({
                    screen: _telemetryScreenId,
                });

                MZAuthInApp.resendVerificationSessionCodeEmail();
                MZErrorHandler.requestAlert(MZErrorHandler.AuthCodeSentAlert);
            }
        }

        MZCancelButton {
            objectName: authSignUp.objectName + "-cancel"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                Glean.interaction.cancelSelected.record({
                    screen: _telemetryScreenId,
                });

                if (isReauthFlow) {
                    cancelAuthenticationFlow();
                } else {
                    VPN.cancelAuthentication();
                }
            }
        }
    }

    Component.onCompleted: {
        Glean.impression.enterVerificationCodeScreen.record({
            screen: _telemetryScreenId,
        });
    }
}
