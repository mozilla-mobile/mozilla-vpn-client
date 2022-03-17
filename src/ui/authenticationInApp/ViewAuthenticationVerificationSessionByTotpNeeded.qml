/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

VPNInAppAuthenticationBase {
    // TODO
    // we are here if the user is completing the session activation flow.
    // The TOTP code is required.
    //
    // After this step we can go to:
    // - all done \o/
    // - error
    //
    // Note that "all done" means IAP! So, more likely, you will receive
    // errors, crashes, a lot of fun things... nothing will work, and more
    // likely the app will implode.
    // In theory, this part should be tested on iOS or on Android when we will
    // have IAP there too.

    id: totpNeeded

    Component.onCompleted: console.log("SESSION VERIFICATION BY TOTP")

    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => { VPN.cancelAuthentication() }
    _menuButtonAccessibleName: qsTrId("vpn.connectionInfo.close")
    _headlineText: VPNl18n.InAppAuthSecurityCodeTitle
    _subtitleText: VPNl18n.InAppAuthSecurityCodeSubtitle
    _imgSource: "qrc:/nebula/resources/verification-code.svg"
    _inputLabel: VPNl18n.InAppAuthSecurityCodeLabel

    _inputs: VPNInAppAuthenticationInputs {
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateVerificationSessionByTotpNeeded && !activeInput().hasError
        _buttonOnClicked: (inputText) => { VPNAuthInApp.verifySessionTotpCode(inputText) }
        _buttonText: VPNl18n.InAppAuthVerifySecurityCodeButton
        _inputMethodHints: Qt.ImhDigitsOnly
        _inputPlaceholderText: VPNl18n.InAppAuthVerificationCodeInputPlaceholder
        _itemToPan: totpNeeded
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter

        VPNInAppAuthenticationCancel {}
    }
}
