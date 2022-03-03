/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

ColumnLayout {
    id: base
    property string _inputPlaceholderText: ""
    property string _inputErrorMessage: ""
    property alias _inputMethodHints: textInput.inputMethodHints
    property var _buttonOnClicked
    property alias _buttonEnabled: btn.enabled
    property alias _buttonText: btn.text
    property bool _isSignInView: (VPNAuthInApp.state === VPNAuthInApp.StateSignIn || VPNAuthInApp.state === VPNAuthInApp.StateSigningIn)

    function activeInput() {
        return _isSignInView ? passwordInput : textInput
    }

    Component.onCompleted: if (typeof(authError) === "undefined" || !authError.visible) activeInput().forceActiveFocus();

    spacing: VPNTheme.theme.vSpacing - VPNTheme.theme.listSpacing

    ColumnLayout {
        id: col
        Layout.fillWidth: true
        spacing: VPNTheme.theme.listSpacing

        function submitInfo(input) {
            if (!input.hasError && input.text.length > 0) btn.clicked();
        }

        VPNTextField {
            id: textInput
            Layout.fillWidth: true
            _placeholderText: _inputPlaceholderText
            Keys.onReturnPressed: col.submitInfo(textInput)
            onTextChanged: if (hasError) hasError = false
        }

        VPNPasswordInput {
            id: passwordInput
            _placeholderText: _inputPlaceholderText
            Layout.fillWidth: true
            Keys.onReturnPressed: col.submitInfo(passwordInput)
            onTextChanged: if (hasError) hasError = false
        }

        VPNContextualAlerts {
            anchors.left: undefined
            anchors.right: undefined
            anchors.topMargin: undefined
            Layout.minimumHeight: VPNTheme.theme.vSpacing
            Layout.fillHeight: false
            messages: [
                {
                    type: "error",
                    message: base._inputErrorMessage,
                    visible: activeInput().hasError
                }
            ]
        }
    }

    states: [
        State {
            when: _isSignInView
            PropertyChanges {
                target: textInput
                visible: false
            }
            PropertyChanges {
                target: passwordInput
                visible: true
            }
        },
        State {
            when: !_isSignInView
            PropertyChanges {
                target: textInput
                visible: true
            }
            PropertyChanges {
                target: passwordInput
                visible: false
            }
        }
    ]

    VPNButton {
        id: btn
        Layout.fillWidth: true
        loaderVisible:  VPNAuthInApp.state === VPNAuthInApp.StateCheckingAccount ||
                        VPNAuthInApp.state === VPNAuthInApp.StateSigningIn ||
                        VPNAuthInApp.state === VPNAuthInApp.StateSigningUp ||
                        VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionEmailCode ||
                        VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionTotpCode
        onClicked: _buttonOnClicked(activeInput().text)

    }

    Connections {
        target: VPNAuthInApp
        function onErrorOccurred(e) {
            switch(e) {
            case VPNAuthInApp.ErrorIncorrectPassword:
                base._inputErrorMessage =  VPNl18n.InAppAuthInvalidPasswordErrorMessage;
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidEmailCode:
                base._inputErrorMessage = "Invalid email code";
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidOrExpiredVerificationCode:
                base._inputErrorMessage = "Invalid or expired verification code"
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidTotpCode:
                base._inputErrorMessage = "invalid 2fa unblock code";
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidUnblockCode:
                base._inputErrorMessage = "Invalid unblock code";
                activeInput().forceActiveFocus();
                break;
            }

            if (!authError.visible)
                activeInput().forceActiveFocus();
            activeInput().hasError = true;
        }
    }
}
