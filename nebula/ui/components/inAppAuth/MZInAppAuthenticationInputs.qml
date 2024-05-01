/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import compat 0.1

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

ColumnLayout {
    id: base
    property string _viewObjectName: ""
    property string _telemetryScreenId
    property string _telemetryButtonEventName
    property string _inputPlaceholderText: ""
    property string _inputErrorMessage: ""
    property alias _inputMethodHints: textInput.inputMethodHints
    property var _buttonOnClicked
    property alias _buttonEnabled: btn.enabled
    property alias _buttonText: btn.text
    property bool _isSignUpOrIn: MZAuthInApp.state === MZAuthInApp.StateSignIn || MZAuthInApp.state === MZAuthInApp.StateSigningIn || MZAuthInApp.state === MZAuthInApp.StateSignUp || MZAuthInApp.state === MZAuthInApp.StateSigningUp

    function activeInput() {
        return _isSignUpOrIn ? passwordInput : textInput
    }

    function disableActiveInput() {
        activeInput().enabled = false;
    }

    Component.onCompleted: if (typeof(authError) === "undefined" || !authError.visible) activeInput().forceActiveFocus();

    spacing: MZTheme.theme.vSpacing - MZTheme.theme.listSpacing
    Layout.preferredWidth: parent.width

    ColumnLayout {
        function submitInfo(input) {
            if (!input.hasError && input.text.length > 0 && btn.enabled)
            {
                disableActiveInput();
                btn.clicked();
            }
        }

        id: col

        spacing: MZTheme.theme.listSpacing
        Layout.preferredWidth: parent.width

        Row {
            spacing: MZTheme.theme.windowMargin / 2

            MZTextField {
                id: textInput
                objectName: base._viewObjectName + "-textInput"
                width: base.width - (inputPasteButton.visible ? inputPasteButton.width - parent.spacing : 0)
                height: MZTheme.theme.rowHeight
                _placeholderText: _inputPlaceholderText
                Keys.onReturnPressed: col.submitInfo(textInput)
                onTextChanged: if (hasError) hasError = false
            }

            MZPasswordInput {
                id: passwordInput
                objectName: base._viewObjectName + "-passwordInput"
                _placeholderText: _inputPlaceholderText
                Keys.onReturnPressed: col.submitInfo(passwordInput)
                width: base.width - inputPasteButton.width - parent.spacing
                height: MZTheme.theme.rowHeight
                onTextChanged: if (hasError) hasError = false
            }

            MZPasteButton {
                id: inputPasteButton
                objectName: base._viewObjectName + "-inputPasteButton"
                width: MZTheme.theme.rowHeight
                height: MZTheme.theme.rowHeight
                onClicked: {
                    Glean.interaction.pastePasswordSelected.record({
                        screen: _telemetryScreenId,
                    });

                   activeInput().paste();
                   activeInput().forceActiveFocus();
                }
            }
        }


        ToolTip {
            property bool _isSignUp: MZAuthInApp.state === MZAuthInApp.StateSignUp
            id: toolTip
            visible: _isSignUp && passwordInput.text.length > 0 && passwordInput.activeFocus
            padding: MZTheme.theme.windowMargin
            x: MZTheme.theme.vSpacing
            y: passwordInput.y - height - 4
            width: passwordInput.width - MZTheme.theme.vSpacing
            height: passwordConditions.implicitHeight + padding * 2
            background: Rectangle { color: MZTheme.theme.transparent }

            Rectangle {
                id: bg
                anchors.fill: passwordConditions
                anchors.margins: MZTheme.theme.windowMargin * -1
                color: MZTheme.colors.white
                radius: MZTheme.theme.cornerRadius

                MZRectangularGlow {
                    anchors.fill: glowVector
                    glowRadius: 4
                    spread: .3
                    color: MZTheme.theme.divider
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
                    anchors.rightMargin: MZTheme.theme.windowMargin
                    width: MZTheme.theme.windowMargin / 2
                    height: MZTheme.theme.windowMargin / 2
                    color: parent.color
                    rotation: 45
                }
            }

            ColumnLayout {
                id: passwordConditions
                spacing: MZTheme.theme.windowMargin / 2
                anchors.right: parent.right
                width: parent.width

                MZInAppAuthenticationPasswordCondition {
                    id: passwordLength
                    objectName: base._viewObjectName + "-passwordConditionLength"
                    _iconVisible: true
                    _passwordConditionIsSatisfied: toolTip._isSignUp && MZAuthInApp.validatePasswordLength(passwordInput.text)
                    _passwordConditionDescription: MZI18n.InAppAuthPasswordHintCharacterLength
                }
                MZInAppAuthenticationPasswordCondition {
                    objectName: base._viewObjectName + "-passwordConditionEmailAddress"
                    _iconVisible: passwordLength._passwordConditionIsSatisfied
                    _passwordConditionIsSatisfied: toolTip._isSignUp && passwordLength._passwordConditionIsSatisfied && MZAuthInApp.validatePasswordEmail(passwordInput.text)
                    _passwordConditionDescription: MZI18n.InAppAuthPasswordHintEmailAddressAsPassword
                    opacity: passwordLength._passwordConditionIsSatisfied ? 1 : .5
                }
                MZInAppAuthenticationPasswordCondition {
                    objectName: base._viewObjectName + "-passwordConditionCommon"
                    _iconVisible:  passwordLength._passwordConditionIsSatisfied
                    _passwordConditionIsSatisfied: toolTip._isSignUp && passwordLength._passwordConditionIsSatisfied && MZAuthInApp.validatePasswordCommons(passwordInput.text)
                    _passwordConditionDescription: MZI18n.InAppAuthPasswordHintCommonPassword
                    opacity: _iconVisible ? 1 : .5
                }
            }
        }

        MZContextualAlerts {
            id: inputErrors
            anchors.left: undefined
            anchors.right: undefined
            anchors.topMargin: undefined
            Layout.minimumHeight: MZTheme.theme.vSpacing
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
            when: MZAuthInApp.state === MZAuthInApp.StateStart ||
                  MZAuthInApp.state === MZAuthInApp.StateCheckingAccount
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
            when: MZAuthInApp.state === MZAuthInApp.StateUnblockCodeNeeded ||
                  MZAuthInApp.state === MZAuthInApp.StateVerifyingUnblockCode ||
                  MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByEmailNeeded ||
                  MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionEmailCode ||
                  MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByTotpNeeded ||
                  MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionTotpCode
            extend: "auth-start"
            PropertyChanges {
                target: inputPasteButton
                visible: true
            }
        },

        State {
            when: MZAuthInApp.state === MZAuthInApp.StateSignUp || MZAuthInApp.state === MZAuthInApp.StateSigningUp ||
                  MZAuthInApp.state === MZAuthInApp.StateSignIn || MZAuthInApp.state === MZAuthInApp.StateSigningIn
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

    MZButton {
        id: btn
        objectName: base._viewObjectName + "-button"
        Layout.fillWidth: true
        loaderVisible:  MZAuthInApp.state === MZAuthInApp.StateCheckingAccount ||
                        MZAuthInApp.state === MZAuthInApp.StateSigningIn ||
                        MZAuthInApp.state === MZAuthInApp.StateSigningUp ||
                        MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionEmailCode ||
                        MZAuthInApp.state === MZAuthInApp.StateVerifyingSessionTotpCode
        onClicked: {
            Glean.interaction[_telemetryButtonEventName].record({
                screen: _telemetryScreenId,
            });

            disableActiveInput();
            _buttonOnClicked(activeInput().text);
        }
        width: undefined

    }


    Connections {
        target: MZAuthInApp
        function onErrorOccurred(e, retryAfter) {
            switch(e) {
            case MZAuthInApp.ErrorIncorrectPassword:
                Glean.sample.authenticationError.record({ reason: "IncorrectPassword" });

                base._inputErrorMessage =  MZI18n.InAppAuthInvalidPasswordErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case MZAuthInApp.ErrorInvalidEmailAddress:
                Glean.sample.authenticationError.record({ reason: "InvalidEmail" });

                base._inputErrorMessage =  MZI18n.InAppAuthInvalidEmailErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case MZAuthInApp.ErrorInvalidOrExpiredVerificationCode:
                Glean.sample.authenticationError.record({ reason: "InvalidOrExpiredVerificationCode" });

                base._inputErrorMessage = MZI18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;

            case MZAuthInApp.ErrorInvalidTotpCode:
                Glean.sample.authenticationError.record({ reason: "InvalidTotpCode" });

                base._inputErrorMessage = MZI18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;

            case MZAuthInApp.ErrorInvalidUnblockCode:
                Glean.sample.authenticationError.record({ reason: "InvalidUnblockCode" });

                base._inputErrorMessage = MZI18n.InAppAuthInvalidCodeErrorMessage;
                activeInput().forceActiveFocus();
                break;
            case MZAuthInApp.ErrorConnectionTimeout:
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
