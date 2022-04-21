/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

VPNInAppAuthenticationBase {
    property bool isDeleteAccountAuth: false

    _changeEmailLinkVisible: !isDeleteAccountAuth
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        if (isDeleteAccountAuth) {
            VPN.cancelAccountDeletion();
        }
        VPNAuthInApp.reset();
    }
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: !isDeleteAccountAuth
        ? VPNl18n.InAppAuthSignInSubtitle
        : VPNl18n.DeleteAccountAuthSubheadline
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: VPNl18n.InAppAuthPasswordInputLabel

    _inputs: VPNInAppAuthenticationInputs {
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateSignIn && !activeInput().hasError
        _buttonOnClicked: (inputText) => {
             VPNAuthInApp.setPassword(inputText);
             VPNAuthInApp.signIn();
         }
        _buttonText: !isDeleteAccountAuth
            ? VPNl18n.InAppAuthSignInButton
            : VPNl18n.DeleteAccountAuthButtonLabel
        _inputPlaceholderText: VPNl18n.InAppAuthPasswordInputPlaceholder
    }

    _disclaimers: Loader {
        id: disclaimersLoader
        source: !isDeleteAccountAuth
            ? "qrc:/nebula/components/inAppAuth/VPNInAppAuthenticationLegalDisclaimer.qml"
            : "";
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNLinkButton {
            labelText: VPNl18n.InAppAuthForgotPasswordLink
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPN.openLink(VPN.LinkForgotPassword)
        }

        VPNCancelButton {
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                if (isDeleteAccountAuth) {
                    VPN.cancelAccountDeletion();
                }
                VPN.cancelAuthentication();
            }
        }
    }
}
