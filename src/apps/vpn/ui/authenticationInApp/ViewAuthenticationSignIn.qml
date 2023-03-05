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
    id: authSignIn

    states:[
        State {
            when: isReauthFlow

            PropertyChanges {
                target: authSignIn

                _changeEmailLinkVisible: false
                _subtitleText: MZI18n.InAppAuthReauthSignInSubtitle
            }

            PropertyChanges {
                target: disclaimersLoader

                source: ""
            }

            PropertyChanges {
                target: authInputs

                _buttonText: MZI18n.DeleteAccountAuthButtonLabel
            }
        }
    ]

    _changeEmailLinkVisible: true
    _viewObjectName: "authSignIn"
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonImageMirror: MZLocalizer.isRightToLeft
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            cancelAuthenticationFlow();
        } else {
            VPNAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: MZI18n.InAppAuthSignInSubtitle
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: MZI18n.InAppAuthPasswordInputLabel

    _inputs: MZInAppAuthenticationInputs {
        objectName: "authSignIn"
        id: authInputs

        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateSignIn && !activeInput().hasError
        _buttonOnClicked: (inputText) => {
             VPNAuthInApp.setPassword(inputText);
             VPNAuthInApp.signIn();
         }
        _buttonText: MZI18n.InAppAuthSignInButton
        _inputPlaceholderText: MZI18n.InAppAuthPasswordInputPlaceholder
    }

    _disclaimers: Loader {
        id: disclaimersLoader

        Layout.alignment: Qt.AlignHCenter
        source: "qrc:/nebula/components/inAppAuth/MZInAppAuthenticationLegalDisclaimer.qml"
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: MZTheme.theme.windowMargin

        MZLinkButton {
            labelText: MZI18n.InAppAuthForgotPasswordLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: MZUrlOpener.openUrlLabel("forgotPassword")
        }

        MZCancelButton {
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
