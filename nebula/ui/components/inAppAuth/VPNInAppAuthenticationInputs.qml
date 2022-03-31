/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import compat 0.1

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
    property bool _isSignUpOrIn: VPNAuthInApp.state === VPNAuthInApp.StateSignIn || VPNAuthInApp.state === VPNAuthInApp.StateSigningIn || VPNAuthInApp.state === VPNAuthInApp.StateSignUp || VPNAuthInApp.state === VPNAuthInApp.StateSigningUp

    function activeInput() {
        return _isSignUpOrIn ? passwordInput : textInput
    }

    Component.onCompleted: if (typeof(authError) === "undefined" || !authError.visible) activeInput().forceActiveFocus();

    spacing: VPNTheme.theme.vSpacing - VPNTheme.theme.listSpacing

    ColumnLayout {
        function submitInfo(input) {
            if (!input.hasError && input.text.length > 0 && btn.enabled) btn.clicked();
        }

        id: col

        spacing: VPNTheme.theme.listSpacing

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

        ToolTip {
            property bool _isSignUp: VPNAuthInApp.state === VPNAuthInApp.StateSignUp
            id: toolTip
            visible: _isSignUp && passwordInput.activeFocus
            padding: VPNTheme.theme.windowMargin
            x: VPNTheme.theme.vSpacing
            y: passwordInput.y - height - 4
            width: passwordInput.width - VPNTheme.theme.vSpacing
            height: passwordConditions.implicitHeight + padding * 2

            background: Rectangle { color: VPNTheme.theme.transparent }

            Rectangle {
                id: bg
                anchors.fill: passwordConditions
                anchors.margins: VPNTheme.theme.windowMargin * -1
                color: VPNTheme.colors.white
                radius: VPNTheme.theme.cornerRadius

                VPNRectangularGlow {
                    anchors.fill: glowVector
                    glowRadius: 4
                    spread: .3
                    color: VPNTheme.theme.divider
                    cornerRadius: glowVector.radius + glowRadius
                    z: -2
                }

                Rectangle {
                    id: glowVector
                    anchors.fill: parent
                    radius: bg.radius
                    color: bg.color
                }

                Rectangle {
                    radius: 1
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -3
                    anchors.right: parent.right
                    anchors.rightMargin: VPNTheme.theme.windowMargin
                    width: VPNTheme.theme.windowMargin / 2
                    height: VPNTheme.theme.windowMargin / 2
                    color: parent.color
                    rotation: 45
                }
            }

            ColumnLayout {
                id: passwordConditions
                spacing: VPNTheme.theme.windowMargin / 2
                anchors.right: parent.right
                width: parent.width

                VPNInAppAuthenticationPasswordCondition {
                    _passwordConditionIsSatisfied: toolTip._isSignUp && VPNAuthInApp.validatePasswordLength(passwordInput.text)
                    _passwordConditionDescription:  VPNl18n.InAppAuthPasswordHintCharacterLength
                }
                VPNInAppAuthenticationPasswordCondition {
                    _passwordConditionIsSatisfied: toolTip._isSignUp && VPNAuthInApp.validatePasswordEmail(passwordInput.text)
                    _passwordConditionDescription:  VPNl18n.InAppAuthPasswordHintEmailAddressAsPassword
                }
                VPNInAppAuthenticationPasswordCondition {
                    _passwordConditionIsSatisfied: toolTip._isSignUp && passwordInput.text.length > 0 && VPNAuthInApp.validatePasswordCommons(passwordInput.text)
                    _passwordConditionDescription:  VPNl18n.InAppAuthPasswordHintCommonPassword
                }
            }
        }

        VPNContextualAlerts {
            id: inputErrors
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
            when: _isSignUpOrIn
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
            when: !_isSignUpOrIn
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
        width: undefined

    }


    Connections {
        target: VPNAuthInApp
        function onErrorOccurred(e, retryAfter) {
            switch(e) {
            case VPNAuthInApp.ErrorIncorrectPassword:
                base._inputErrorMessage =  VPNl18n.InAppAuthInvalidPasswordErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case VPNAuthInApp.ErrorInvalidEmailAddress:
                base._inputErrorMessage =  VPNl18n.InAppAuthInvalidEmailErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case VPNAuthInApp.ErrorInvalidEmailCode:
                base._inputErrorMessage = VPNl18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidOrExpiredVerificationCode:
                base._inputErrorMessage = VPNl18n.InAppAuthInvalidCodeErrorMessage
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidTotpCode:
                base._inputErrorMessage = VPNl18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidUnblockCode:
                base._inputErrorMessage = VPNl18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;
            }

            if (!authError.visible)
                activeInput().forceActiveFocus();
            activeInput().hasError = true;
        }
    }
}
