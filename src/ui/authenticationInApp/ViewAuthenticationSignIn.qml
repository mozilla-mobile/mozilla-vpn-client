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
    objectName: "authSignIn"

    _telemetryScreenId: "enter_password"

    states:[
        State {
            when: isReauthFlow

            PropertyChanges {
                target: authSignIn

                _changeEmailLinkVisible: false
                _subtitleText: MZI18n.InAppAuthReauthSignInSubtitle2
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
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonImageMirror: MZLocalizer.isRightToLeft
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            // No telemetry.
            // Re-auth flow is not contemplated by the telemetry design.

            cancelAuthenticationFlow();
        } else {
            Glean.interaction.backSelected.record({
                screen: authSignIn._telemetryScreenId,
            });

            MZAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: MZI18n.GlobalGoBack
    _headlineText: MZAuthInApp.emailAddress
    _subtitleText: MZI18n.InAppAuthSignInSubtitle2
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: MZI18n.InAppAuthPasswordInputLabel

    _inputs: MZInAppAuthenticationInputs {
        id: authInputs
        _viewObjectName: authSignIn.objectName

        _telemetryScreenId: authSignIn._telemetryScreenId
        _telemetryButtonEventName: "signInSelected"

        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateSignIn && !activeInput().hasError
        _buttonOnClicked: (inputText) => {
             MZAuthInApp.setPassword(inputText);
             MZAuthInApp.signIn();
         }
        _buttonText: MZI18n.InAppAuthSignInButton
        _inputPlaceholderText: MZI18n.InAppAuthPasswordInputPlaceholder
    }

    _disclaimers: Loader {
        id: disclaimersLoader

        Layout.alignment: Qt.AlignHCenter

        Component.onCompleted: {
            disclaimersLoader.setSource(
                "qrc:/nebula/components/inAppAuth/MZInAppAuthenticationLegalDisclaimer.qml",
                { "_telemetryScreenId": authSignIn._telemetryScreenId }
            );
        }
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: parent.width
        spacing: MZTheme.theme.windowMargin

        MZLinkButton {
            objectName: authSignIn.objectName + "-forgotPassword"
            labelText: MZI18n.InAppAuthForgotPasswordLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                Glean.interaction.forgotYourPasswordSelected.record({
                    screen: _telemetryScreenId,
                });

                MZUrlOpener.openUrlLabel("forgotPassword")
            }
        }

        MZCancelButton {
            objectName: authSignIn.objectName + "-cancel"
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
        Glean.impression.enterPasswordScreen.record({
            screen: _telemetryScreenId,
        });
    }
}
