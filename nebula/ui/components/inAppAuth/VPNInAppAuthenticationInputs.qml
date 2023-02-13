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

    function disableActiveInput() {
        activeInput().enabled = false;
    }

    Component.onCompleted: if (typeof(authError) === "undefined" || !authError.visible) activeInput().forceActiveFocus();

    spacing: VPNTheme.theme.vSpacing - VPNTheme.theme.listSpacing

    ColumnLayout {
        function submitInfo(input) {
            if (!input.hasError && input.text.length > 0 && btn.enabled)
            {
                disableActiveInput();
                btn.clicked();
            }
        }

        id: col

        spacing: VPNTheme.theme.listSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: VPNTheme.theme.windowMargin / 2

            VPNTextField {
                id: textInput
                objectName: base.objectName + "-textInput"
                Layout.fillWidth: true
                _placeholderText: _inputPlaceholderText
                Keys.onReturnPressed: col.submitInfo(textInput)
                onDisplayTextChanged: if (hasError) hasError = false
            }

            VPNPasswordInput {
                id: passwordInput
                objectName: base.objectName + "-passwordInput"
                _placeholderText: _inputPlaceholderText
                Keys.onReturnPressed: col.submitInfo(passwordInput)
                Layout.fillWidth: true
                onTextChanged: if (hasError) hasError = false
            }

            VPNPasteButton {
                id: inputPasteButton
                objectName: base.objectName + "-inputPasteButton"
                Layout.preferredWidth: VPNTheme.theme.rowHeight
                Layout.preferredHeight: VPNTheme.theme.rowHeight
                height: undefined
                width: undefined
                onClicked: {
                   activeInput().paste();
                   activeInput().forceActiveFocus();
                }
            }
        }


        ToolTip {
            property bool _isSignUp: VPNAuthInApp.state === VPNAuthInApp.StateSignUp
            id: toolTip
            visible: _isSignUp && passwordInput.text.length > 0 && passwordInput.activeFocus
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
                    id: passwordLength
                    objectName: base.objectName + "-passwordConditionLength"
                    _iconVisible: true
                    _passwordConditionIsSatisfied: toolTip._isSignUp && VPNAuthInApp.validatePasswordLength(passwordInput.text)
                    _passwordConditionDescription: VPNI18n.InAppAuthPasswordHintCharacterLength
                }
                VPNInAppAuthenticationPasswordCondition {
                    objectName: base.objectName + "-passwordConditionEmailAddress"
                    _iconVisible: passwordLength._passwordConditionIsSatisfied
                    _passwordConditionIsSatisfied: toolTip._isSignUp && passwordLength._passwordConditionIsSatisfied && VPNAuthInApp.validatePasswordEmail(passwordInput.text)
                    _passwordConditionDescription: VPNI18n.InAppAuthPasswordHintEmailAddressAsPassword
                    opacity: passwordLength._passwordConditionIsSatisfied ? 1 : .5
                }
                VPNInAppAuthenticationPasswordCondition {
                    objectName: base.objectName + "-passwordConditionCommon"
                    _iconVisible:  passwordLength._passwordConditionIsSatisfied
                    _passwordConditionIsSatisfied: toolTip._isSignUp && passwordLength._passwordConditionIsSatisfied && VPNAuthInApp.validatePasswordCommons(passwordInput.text)
                    _passwordConditionDescription: VPNI18n.InAppAuthPasswordHintCommonPassword
                    opacity: _iconVisible ? 1 : .5
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
            name: "auth-start"
            when: VPNAuthInApp.state === VPNAuthInApp.StateStart ||
                  VPNAuthInApp.state === VPNAuthInApp.StateCheckingAccount
            PropertyChanges {
                target: textInput
                visible: true
            }
            PropertyChanges {
                target: inputPasteButton
                visible: false
            }
            PropertyChanges {
                target: passwordInput
                visible: false
            }
        },
        State {
            when: VPNAuthInApp.state === VPNAuthInApp.StateUnblockCodeNeeded ||
                  VPNAuthInApp.state === VPNAuthInApp.StateVerifyingUnblockCode ||
                  VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByEmailNeeded ||
                  VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionEmailCode ||
                  VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByTotpNeeded ||
                  VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionTotpCode
            extend: "auth-start"
            PropertyChanges {
                target: inputPasteButton
                visible: true
            }
        },

        State {
            when: VPNAuthInApp.state === VPNAuthInApp.StateSignUp || VPNAuthInApp.state === VPNAuthInApp.StateSigningUp ||
                  VPNAuthInApp.state === VPNAuthInApp.StateSignIn || VPNAuthInApp.state === VPNAuthInApp.StateSigningIn
            PropertyChanges {
                target: textInput
                visible: false
            }
            PropertyChanges {
                target: passwordInput
                visible: true
            }
        }
    ]

    VPNButton {
        id: btn
        objectName: base.objectName + "-button"
        Layout.fillWidth: true
        loaderVisible:  VPNAuthInApp.state === VPNAuthInApp.StateCheckingAccount ||
                        VPNAuthInApp.state === VPNAuthInApp.StateSigningIn ||
                        VPNAuthInApp.state === VPNAuthInApp.StateSigningUp ||
                        VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionEmailCode ||
                        VPNAuthInApp.state === VPNAuthInApp.StateVerifyingSessionTotpCode
        onClicked: {
            disableActiveInput();
            _buttonOnClicked(activeInput().text);
        }
        width: undefined

    }


    Connections {
        target: VPNAuthInApp
        function onErrorOccurred(e, retryAfter) {
            switch(e) {
            case VPNAuthInApp.ErrorIncorrectPassword:
                VPNGleanDeprecated.recordGleanEventWithExtraKeys("authenticationError", { "reason": "IncorrectPassword" });
                Glean.sample.authenticationError.record({ reason: "IncorrectPassword" });

                base._inputErrorMessage =  VPNl18n.InAppAuthInvalidPasswordErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case VPNAuthInApp.ErrorInvalidEmailAddress:
                VPNGleanDeprecated.recordGleanEventWithExtraKeys("authenticationError", { "reason": "InvalidEmail" });
                Glean.sample.authenticationError.record({ reason: "InvalidEmail" });

                base._inputErrorMessage =  VPNl18n.InAppAuthInvalidEmailErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case VPNAuthInApp.ErrorInvalidOrExpiredVerificationCode:
                VPNGleanDeprecated.recordGleanEventWithExtraKeys("authenticationError", { "reason": "InvalidOrExpiredVerificationCode" });
                Glean.sample.authenticationError.record({ reason: "InvalidOrExpiredVerificationCode" });

                base._inputErrorMessage = VPNl18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidTotpCode:
                VPNGleanDeprecated.recordGleanEventWithExtraKeys("authenticationError", { "reason": "InvalidTotpCode" });
                Glean.sample.authenticationError.record({ reason: "InvalidTotpCode" });

                base._inputErrorMessage = VPNl18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;

            case VPNAuthInApp.ErrorInvalidUnblockCode:
                VPNGleanDeprecated.recordGleanEventWithExtraKeys("authenticationError", { "reason": "InvalidUnblockCode" });
                Glean.sample.authenticationError.record({ reason: "InvalidUnblockCode" });

                base._inputErrorMessage = VPNl18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case VPNAuthInApp.ErrorConnectionTimeout:
                VPNGleanDeprecated.recordGleanEventWithExtraKeys("authenticationError", { "reason": "Timeout" });
                Glean.sample.authenticationError.record({ reason: "Timeout" });

                // In case of a timeout we want to exit here 
                // to skip setting hasError - so the user can retry instantly
                activeInput().enabled = true;
                return;
            }

            if (!authError.visible)
                activeInput().forceActiveFocus();
            activeInput().hasError = true;
            activeInput().enabled = true;
        }
    }
}
