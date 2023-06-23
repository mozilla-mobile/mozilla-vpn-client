/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1


MZInAppAuthenticationBase {
    _changeEmailLinkVisible: true
    _viewObjectName: "authSignUp"
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonImageMirror: MZLocalizer.isRightToLeft
    _menuButtonOnClick: () => {
        MZAuthInApp.reset();
    }
    _menuButtonAccessibleName: MZI18n.GlobalGoBack
    _headlineText: MZAuthInApp.emailAddress
    _subtitleText: MZI18n.InAppAuthFinishAccountCreationDescription
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: MZI18n.InAppAuthCreatePasswordLabel

    _inputs: MZInAppAuthenticationInputs {
        objectName: "authSignUp"

        function validatePassword(passwordString) {
            return MZAuthInApp.validatePasswordCommons(passwordString)
                && MZAuthInApp.validatePasswordLength(passwordString)
                && MZAuthInApp.validatePasswordEmail(passwordString)
        }
        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateSignUp && validatePassword(activeInput().text)
        _buttonOnClicked: (inputText) => {
                              MZAuthInApp.setPassword(inputText);
                              MZAuthInApp.signUp();
                          }
        _buttonText: MZI18n.InAppAuthCreateAccountButton
        _inputPlaceholderText: MZI18n.InAppAuthPasswordInputPlaceholder
    }

    _disclaimers: Column {
        Layout.alignment: Qt.AlignHCenter
        MZInAppAuthenticationLegalDisclaimer {}
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter

        MZCancelButton {
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: VPN.cancelAuthentication()
        }
    }
}
