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

    property var _itemToPan
    property bool keyboardIsVisible: false

    function activeInput() {
        return _isSignUpOrIn ? passwordInput : textInput
    }

    Component.onCompleted: if (typeof(authError) === "undefined" || !authError.visible) activeInput().forceActiveFocus();

    spacing: VPNTheme.theme.vSpacing - VPNTheme.theme.listSpacing

    ColumnLayout {
        function submitInfo(input) {
            if (!input.hasError && input.text.length > 0) btn.clicked();
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

            ToolTip {
                property bool _isSignUp: VPNAuthInApp.state === VPNAuthInApp.StateSignUp
                id: toolTip
                visible: _isSignUp && passwordInput.activeFocus
                padding: VPNTheme.theme.windowMargin
                x: VPNTheme.theme.vSpacing
                width: passwordInput.width - VPNTheme.theme.vSpacing

                background: Rectangle {
                    id: bg
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

                contentItem: ColumnLayout {
                    spacing: VPNTheme.theme.windowMargin / 2

                    VPNInAppAuthenticationPasswordCondition {
                        _passwordConditionIsSatisfied: toolTip._isSignUp && VPNAuthInApp.validatePasswordLength(passwordInput.text)
                        _passwordConditionDescription:  "Must be a minumum of 8 characters and lots of wrapping text"
                    }
                    VPNInAppAuthenticationPasswordCondition {
                        _passwordConditionIsSatisfied: toolTip._isSignUp && VPNAuthInApp.validatePasswordEmail(passwordInput.text)
                        _passwordConditionDescription:  "Must not be email"
                    }
                    VPNInAppAuthenticationPasswordCondition {
                        _passwordConditionIsSatisfied: toolTip._isSignUp && passwordInput.text.length > 0 && VPNAuthInApp.validatePasswordCommons(passwordInput.text)
                        _passwordConditionDescription:  "Must not be a common password"
                    }
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
                base._inputErrorMessage = "Invalid email address";
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

    function handlePanInputIntoView() {
        if (!_itemToPan) {
            return;
        }

        const keyboardHeight = Qt.inputMethod.keyboardRectangle.height;
        const activeInputItem = activeInput();
        const cursorRectangle = _itemToPan.contentItem.mapFromItem(
            activeInputItem,
            activeInputItem.cursorRectangle.x,
            activeInputItem.cursorRectangle.y
        );

        const activeInputBottom = cursorRectangle.y + activeInputItem.cursorRectangle.height;
        const distanceToViewportBottom = _itemToPan.height - activeInputBottom;
        const minOverlapClearance = VPNTheme.theme.rowHeight;
        const overlapVertical = distanceToViewportBottom - keyboardHeight - minOverlapClearance;
        const keyboardWillIntesectWithInput = overlapVertical < 0;

        if (keyboardIsVisible && keyboardWillIntesectWithInput) {
            _itemToPan.y = -1 * Math.abs(overlapVertical);
        } else if (!keyboardIsVisible) {
            _itemToPan.y = 0;
        }
    }

    Connections {
        target: Qt.inputMethod
        enabled: Qt.platform.os === "android"

        function onKeyboardRectangleChanged() {
            handlePanInputIntoView();
        }

        function onVisibleChanged() {
            keyboardIsVisible = !keyboardIsVisible;
        }
    }

}
