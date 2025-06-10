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
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZInAppAuthenticationBase {
    id: authSignUp
    objectName: "authSignUp"

    _changeEmailLinkVisible: true
    _menuButtonImageSource: MZAssetLookup.getImageSource("ArrowBack")
    _menuButtonImageMirror: MZLocalizer.isRightToLeft
    _menuButtonOnClick: () => {
        MZAuthInApp.reset();
    }
    _menuButtonAccessibleName: MZI18n.GlobalGoBack
    _headlineText: MZAuthInApp.emailAddress
    _subtitleText: MZI18n.InAppAuthFinishAccountCreationDescription2
    _imgSource: MZAssetLookup.getImageSource("Avatar")
    _inputLabel: MZI18n.InAppAuthCreatePasswordLabel

    _inputs: MZInAppAuthenticationInputs {
        _viewObjectName: authSignUp.objectName

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
            objectName: authSignUp.objectName + "-cancel"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                VPN.cancelAuthentication()
            }
        }
    }
}
