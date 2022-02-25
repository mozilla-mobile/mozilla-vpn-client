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
    // We are completing an authentication using an existing account.
    // There is nothing to do here, except... waiting.
    // There 5 possible next-steps:
    // - authentication completed (VPN.state will change, the authentication is
    //   completed)
    // - unblock code needed. This can happen for security reasons. We go
    //   to UnblockCodeNeeded. The user needs to insert the 6-digit code.
    // - The user enters the wrong password, sees error.
    // - The user clicks "Change email" or the back arrow and goes back to start.
    // - Some other error, goes back to start and sees error.

    // TODOs (likely there are more)
    // Open forgot password flow in webview on click
    // Form interaction polish
    // Add password criteria tooltip
    // Maybe add button loader

    _changeEmailLinkVisible: true
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => { VPNAuthInApp.reset() }
    _menuButtonAccessibleName: "Back"
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: VPNl18n.InAppAuthSignInSubtitle
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: VPNl18n.InAppAuthPasswordInputLabel

    _inputs: ColumnLayout {
        spacing: VPNTheme.theme.vSpacing - VPNTheme.theme.listSpacing

        ColumnLayout {
            Layout.fillWidth: true
            spacing: VPNTheme.theme.listSpacing

            VPNPasswordInput {
                id: passwordInput
                Layout.fillWidth: true
                _placeholderText: VPNl18n.InAppAuthPasswordInputPlaceholder
                Keys.onReturnPressed: if (!hasError && text.length > 0) signInBtn.clicked();
                onTextChanged: if (passwordInput.hasError) hasError = false
            }

            VPNContextualAlerts {
                id: searchWarning
                anchors.left: undefined
                anchors.right: undefined
                anchors.topMargin: undefined
                Layout.minimumHeight: VPNTheme.theme.vSpacing
                Layout.fillHeight: false
                messages: [
                    {
                        type: "error",
                        message: VPNl18n.InAppAuthInvalidPasswordErrorMessage,
                        visible: passwordInput.hasError
                    }
                ]
            }
        }

        VPNButton {
            id: signInBtn
            text: VPNl18n.InAppAuthSignInButton
            enabled: VPNAuthInApp.state === VPNAuthInApp.StateSignIn
            loaderVisible: VPNAuthInApp.state === VPNAuthInApp.StateSigningIn
            onClicked: {
                VPNAuthInApp.setPassword(passwordInput.text);
                VPNAuthInApp.signIn();
            }
            Layout.fillWidth: true
        }

        Connections {
            target: VPNAuthInApp
            function onErrorOccurred(e) {
                if (e === 2) {
                    passwordInput.hasError = true;
                    passwordInput.forceActiveFocus();
                }
            }
        }
    }

    _disclaimers: ColumnLayout {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true

        GridLayout {
            id: grid
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            columnSpacing: 0
            columns: 3
            Component.onCompleted: if (implicitWidth > window.width) flow = Grid.TopToBottom

            VPNGreyLink {
                id: termsOfService

                // Terms of Service - string defined in VPNAboutUs.qml
                labelText: qsTrId("vpn.aboutUs.tos2")
                Layout.alignment: grid.columns > 1 ? Qt.AlignRight : Qt.AlignHCenter
                textAlignment: grid.columns > 1 ? Text.AlignRight : Text.AlignHCenter
                onClicked: VPN.openLink(VPN.LinkTermsOfService)
            }

            Rectangle {
                width: 4
                height: 4
                radius: 2
                Layout.alignment: Qt.AlignHCenter
                color: VPNTheme.theme.greyLink.defaultColor
                visible: parent.flow != Grid.TopToBottom
                opacity: .8
            }

            VPNGreyLink {
                id: privacyNotice

                // Privacy Notice - string defined in VPNAboutUs.qml
                labelText: qsTrId("vpn.aboutUs.privacyNotice2")
                onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
                textAlignment: grid.columns > 1 ? Text.AlignLeft : Text.AlignHCenter
                Layout.alignment: grid.columns > 1 ? Qt.AlignLeft : Qt.AlignHCenter
            }
        }
    }


    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: VPNl18n.InAppAuthForgotPasswordLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPN.openLink(VPN.LinkForgotPassword)
        }

        VPNLinkButton {
            // TODO create cancel component and use everywhere
            labelText: VPNl18n.InAppSupportWorkflowSupportSecondaryActionText // "Cancel"
            fontName: VPNTheme.theme.fontBoldFamily
            anchors.horizontalCenter: parent.horizontalCenter
            linkColor: VPNTheme.theme.redButton
            onClicked: VPN.cancelAuthentication()
        }

    }
}
