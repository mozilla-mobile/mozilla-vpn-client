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

    Component.onCompleted: console.log("SESSION VERIFICATION BY TOTP")

    _menuButtonImageSource: "qrc:/nebula/resources/close-dark.svg"
    _menuButtonOnClick: () => { VPNAuthInApp.reset() }
    _menuButtonAccessibleName: "TODO: Lorum Ipsum - Back"
    _headlineText: "Enter 2-factor auth code"
    _subtitleText: "Enter your 2-factor auth code"
    _imgSource: "qrc:/ui/resources/logo.svg"
    _inputLabel: "Enter code"

    _inputs: ColumnLayout {
        spacing: VPNTheme.theme.vSpacing * 2
        VPNTextField {
            id: codeInput
            Layout.fillWidth: true
        }

        VPNButton {
            text: "Verify"
            Layout.fillWidth: true
            onClicked: {
              VPNAuthInApp.verifySessionTotpCode(codeInput.text);
            }
        }
    }

    _footerContent: Column {
        Layout.alignment: Qt.AlignHCenter
        VPNLinkButton {
            labelText: "Cancel"
            fontName: VPNTheme.theme.fontBoldFamily
            anchors.horizontalCenter: parent.horizontalCenter
            linkColor: VPNTheme.theme.redButton
            onClicked: VPNAuthInApp.reset()
        }
    }
}
