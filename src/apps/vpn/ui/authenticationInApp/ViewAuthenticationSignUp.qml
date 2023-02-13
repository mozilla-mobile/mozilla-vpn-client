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
    _changeEmailLinkVisible: true
    _viewObjectName: "authSignUp"
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonImageMirror: VPNLocalizer.isRightToLeft
    _menuButtonOnClick: () => {
        VPNAuthInApp.reset();
    }
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _headlineText: VPNAuthInApp.emailAddress
    _subtitleText: VPNI18n.InAppAuthFinishAccountCreationDescription
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: VPNI18n.InAppAuthCreatePasswordLabel

    _inputs: VPNInAppAuthenticationInputs {
        objectName: "authSignUp"

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
        _buttonText: VPNI18n.InAppAuthCreateAccountButton
        _inputPlaceholderText: VPNI18n.InAppAuthPasswordInputPlaceholder
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
