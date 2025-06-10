/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZInAppAuthenticationBase {
    id: authVerificationSessionByTotpNeeded
    objectName: "authVerificationSessionByTotpNeeded"

    _menuButtonImageSource: MZAssetLookup.getImageSource("CloseDark")
    _menuButtonOnClick: () => {
        if (isReauthFlow) {
            cancelAuthenticationFlow();
        } else {
            MZAuthInApp.reset();
        }
    }
    _menuButtonAccessibleName: MZI18n.GlobalClose
    _headlineText: MZI18n.InAppAuthSecurityCodeTitle
    _subtitleText: MZI18n.InAppAuthSecurityCodeSubtitle
    _imgSource: MZAssetLookup.getImageSource("VerificationCode")
    _inputLabel: MZI18n.InAppAuthSecurityCodeLabel
    _backButtonVisible: false

    _inputs: MZInAppAuthenticationInputs {
        _viewObjectName: authVerificationSessionByTotpNeeded.objectName

        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateVerificationSessionByTotpNeeded && activeInput().text.length === MZAuthInApp.totpCodeLength && !activeInput().hasError
        _buttonOnClicked: (inputText) => { MZAuthInApp.verifySessionTotpCode(inputText) }
        _buttonText: MZI18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhDigitsOnly
        _inputPlaceholderText: MZI18n.InAppAuthSecurityCodeInputPlaceholder
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: parent.width

        MZCancelButton {
            objectName: authVerificationSessionByTotpNeeded.objectName + "-cancel"
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
