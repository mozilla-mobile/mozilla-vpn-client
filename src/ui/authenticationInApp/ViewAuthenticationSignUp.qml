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

    id: authSignUp

    _changeEmailLinkVisible: true
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        VPNAuthInApp.reset();
    }
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: VPNl18n.InAppAuthFinishAccountCreationDescription
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: VPNl18n.InAppAuthCreatePasswordLabel

    _inputs: VPNInAppAuthenticationInputs {
        function validatePassword(passwordString) {
            return VPNAuthInApp.validatePasswordCommons(passwordString)
                && VPNAuthInApp.validatePasswordLength(passwordString)
                && VPNAuthInApp.validatePasswordEmail(passwordString)
        }
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateSignUp && validatePassword(activeInput().text)
        _buttonOnClicked: (inputText) => {
                              VPNAuthInApp.setPassword(inputText);
                              VPNAuthInApp.signUp();
                          }
        _buttonText: VPNl18n.InAppAuthCreateAccountButton
        _inputPlaceholderText: VPNl18n.InAppAuthPasswordInputPlaceholder
    }

    _disclaimers: VPNInAppAuthenticationLegalDisclaimer {}

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter

        VPNCancelButton {
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPN.cancelAuthentication()
        }
    }
}
