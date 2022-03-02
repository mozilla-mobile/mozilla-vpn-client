/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1


VPNInAppAuthenticationBase {
    // TODO
    // We show this view when the user is creating an account.
    // Things are happening in background. The next steps are:
    // - Verification by email needed
    // - errors

    property bool signUpEnabled: passwordIsValid(passwordInputCreate.text)

    id: authSignUp

    _changeEmailLinkVisible: true
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        VPNAuthInApp.reset();
    }
    _menuButtonAccessibleName: "Back"
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor"
    _imgSource: "qrc:/nebula/resources/avatar.svg"

    _inputs: ColumnLayout {
        spacing: VPNTheme.theme.vSpacingSmall

        VPNBoldLabel {
            id: passwordLabelCreate
            text: "Create password"
        }

        VPNPasswordInput {
            id: passwordInputCreate
            Layout.fillWidth: true
            _placeholderText: "secure password" // TODO
        }

        VPNBoldLabel {
            id: passwordLabelConfirm
            text: "Confirm password"
        }

        VPNPasswordInput {
            id: passwordInputConfirm
            Layout.fillWidth: true
            _placeholderText: "secure password" // TODO
        }

        VPNContextualAlerts {
            id: passwordInputCreateWarnings
            Layout.fillWidth: true

            messages: [
                {
                    type: "error",
                    message: "Common password",
                    visible: !VPNAuthInApp.validatePasswordCommons(passwordInputCreate.text)
                },
                {
                    type: "error",
                    message: "Password length",
                    visible: !VPNAuthInApp.validatePasswordLength(passwordInputCreate.text)
                },
                {
                    type: "error",
                    message: "Password email",
                    visible: !VPNAuthInApp.validatePasswordEmail(passwordInputCreate.text)
                },
                {
                    type: "error",
                    message: "Passwords do not match",
                    visible: !authSignUp.passwordsMatch()
                }
            ]
        }

        VPNButton {
            id: createAccountButton
            enabled: authSignUp.signUpEnabled && VPNAuthInApp.state === VPNAuthInApp.StateSignUp
            text: "Create account"
            loaderVisible: VPNAuthInApp.state === VPNAuthInApp.StateSigningUp
            Layout.fillWidth: true

            onClicked: {
                if (enabled) {
                    VPNAuthInApp.setPassword(passwordInputCreate.text);
                    VPNAuthInApp.signUp();
                }
            }
        }
    }

    _disclaimers: RowLayout {
        Layout.alignment: Qt.AlignHCenter
        VPNTextBlock {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
        }
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: "Cancel"
            fontName: VPNTheme.theme.fontBoldFamily
            anchors.horizontalCenter: parent.horizontalCenter
            linkColor: VPNTheme.theme.redButton
            onClicked: VPNAuthInApp.reset()
        }

    }

    function passwordsMatch() {
        return passwordInputConfirm.text === passwordInputCreate.text
    }

    function passwordIsValid(passwordString) {
        return VPNAuthInApp.validatePasswordCommons(passwordString)
            && VPNAuthInApp.validatePasswordLength(passwordString)
            && VPNAuthInApp.validatePasswordEmail(passwordString)
            && passwordsMatch();
    }

}
