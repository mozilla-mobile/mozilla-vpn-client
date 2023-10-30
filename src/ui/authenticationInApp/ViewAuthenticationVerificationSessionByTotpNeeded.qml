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
    id: authVerificationSessionByTotpNeeded

    _telemetryScreenId: "enter_security_code"

    _viewObjectName: "authVerificationSessionByTotpNeeded"
    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            // No telemetry.
            // Re-auth flow is not contemplated by the telemetry design.

            cancelAuthenticationFlow();
        } else {
            Glean.interaction.cancelSelected.record({
                screen: _telemetryScreenId,
            });

            MZAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: MZI18n.GlobalClose
    _headlineText: MZI18n.InAppAuthSecurityCodeTitle
    _subtitleText: MZI18n.InAppAuthSecurityCodeSubtitle
    _imgSource: "qrc:/nebula/resources/verification-code.svg"
    _inputLabel: MZI18n.InAppAuthSecurityCodeLabel
    _backButtonVisible: false

    _inputs: MZInAppAuthenticationInputs {
        objectName: "authVerificationSessionByTotpNeeded"

        _telemetryScreenId: authVerificationSessionByTotpNeeded._telemetryScreenId
        _buttonTelemetryId: "verifySelected"
        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByTotpNeeded && activeInput().text.length === MZAuthInApp.totpCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { MZAuthInApp.verifySessionTotpCode(inputText) }
        _buttonText: MZI18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhDigitsOnly
        _inputPlaceholderText: MZI18n.InAppAuthSecurityCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: parent.width

        MZCancelButton {
            objectName: _viewObjectName + "-cancel"
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
        Glean.impression.enterSecurityCodeScreen.record({
            screen: _telemetryScreenId,
        });
    }
}
