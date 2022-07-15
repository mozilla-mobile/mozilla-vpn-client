/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

VPNInAppAuthenticationBase {
    _viewObjectName: "authStart"
    _menuButtonOnClick: () => { VPN.cancelAuthentication() }
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonAccessibleName:  qsTrId("vpn.main.back")
    _headlineText: "Mozilla VPN"
    _subtitleText: VPNl18n.InAppAuthEnterEmailAddressDescription
    _imgSource: "qrc:/ui/resources/logo.svg"
    _inputLabel: VPNl18n.InAppAuthEmailInputPlaceholder

    _inputs: VPNInAppAuthenticationInputs {
        objectName: "authStart"
        _buttonEnabled: VPNAuthInApp.state === VPNAuthInApp.StateStart && activeInput().text.length !== 0 && !activeInput().hasError
        _buttonOnClicked: (inputText) => { VPNAuthInApp.checkAccount(inputText); }
        _buttonText: qsTrId("vpn.postAuthentication.continue")
        _inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
        _inputPlaceholderText: VPNl18n.InAppSupportWorkflowSupportEmailFieldPlaceholder
    }

    _disclaimers: RowLayout {
        spacing: VPNTheme.theme.vSpacing / 2
        VPNIcon {
            source: "qrc:/nebula/resources/shield-green50.svg"
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        }

        VPNTextBlock {
            id: txt
            text: VPNl18n.InAppAuthInformationUsageDisclaimer
            Layout.fillWidth: true
        }
    }
}
