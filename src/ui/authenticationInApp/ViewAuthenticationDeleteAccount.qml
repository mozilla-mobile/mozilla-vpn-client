/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

VPNInAppAuthenticationBase {
    _changeEmailLinkVisible: false
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {}
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _headlineText: VPNUser.email
    _subtitleText: VPNl18n.DeleteAccountAuthSubheadline
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _inputLabel: VPNl18n.InAppAuthPasswordInputLabel

    _inputs: VPNInAppAuthenticationInputs {
        _buttonEnabled: !activeInput().hasError
        _buttonOnClicked: (inputText) => {}
        _buttonText: VPNl18n.DeleteAccountAuthButtonLabel
        _inputPlaceholderText: VPNl18n.InAppAuthPasswordInputPlaceholder
    }

    _footerContent: ColumnLayout {
        Layout.alignment: Qt.AlignHCenter

        spacing: VPNTheme.theme.vSpacing

        VPNLinkButton {
            Layout.fillWidth: true

            labelText: VPNl18n.InAppAuthForgotPasswordLink
            onClicked: VPN.openLink(VPN.LinkForgotPassword)
        }

        VPNLinkButton {
            Layout.fillWidth: true

            fontName: VPNTheme.theme.fontBoldFamily
            // Cancel
            labelText: VPNl18n.InAppSupportWorkflowSupportSecondaryActionText
            linkColor: VPNTheme.theme.redButton
            onClicked: {}
        }

    }
}
