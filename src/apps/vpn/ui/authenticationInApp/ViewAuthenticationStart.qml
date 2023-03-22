/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

MZInAppAuthenticationBase {
    _viewObjectName: "authStart"
    _menuButtonOnClick: () => { VPN.cancelAuthentication() }
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonImageMirror: MZLocalizer.isRightToLeft
    _menuButtonAccessibleName:  MZI18n.GlobalGoBack
    _headlineText: "Mozilla VPN"
    _subtitleText: MZI18n.InAppAuthEnterEmailAddressDescription
    _imgSource: "qrc:/ui/resources/logo.svg"
    _inputLabel: MZI18n.InAppAuthEmailInputPlaceholder

    _inputs: MZInAppAuthenticationInputs {
        objectName: "authStart"
        _buttonEnabled: MZAuthInApp.state === MZAuthInApp.StateStart && activeInput().text.length !== 0 && !activeInput().hasError && MZAuthInApp.validateEmailAddress(activeInput().text)
        _buttonOnClicked: (inputText) => { MZAuthInApp.checkAccount(inputText); }
        _buttonText: qsTrId("vpn.postAuthentication.continue")
        _inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhEmailCharactersOnly | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
        _inputPlaceholderText: MZI18n.InAppSupportWorkflowSupportEmailFieldPlaceholder
    }

    _disclaimers: RowLayout {
        spacing: MZTheme.theme.vSpacing / 2
        MZIcon {
            source: "qrc:/nebula/resources/shield-green50.svg"
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        }

        MZTextBlock {
            id: txt
            text: MZI18n.InAppAuthInformationUsageDisclaimer
            Layout.fillWidth: true
        }
    }
}
