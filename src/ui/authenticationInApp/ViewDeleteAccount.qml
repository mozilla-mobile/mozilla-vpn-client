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
    id: viewDeleteAccount

    _changeEmailLinkVisible: false
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        VPNAuthInApp.reset();
    }
    _headlineText: "Delete Firefox account"
    _imgSource: "qrc:/nebula/resources/avatar.svg"
    _subtitleText: "Your Firefox account (" + VPNAuthInApp.emailAddress + ") is connected to Mozilla products that keep you secure and productive on the web. Please acknowledge that by deleting your account:"

    _inputs: ColumnLayout {
        spacing: VPNTheme.theme.listSpacing

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter
            labelText: "Any paid subscriptions you have will be cancelled (Except Pocket)"
            leftMargin: 0
            isChecked: boxIsChecked
            showDivider: false
            width: parent.width - VPNTheme.theme.windowMargin * 2

            onClicked: boxIsChecked = !boxIsChecked
        }

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter

            labelText: "You may lose saved information and features within Mozilla products"
            leftMargin: 0
            isChecked: boxIsChecked
            showDivider: false
            width: parent.width - VPNTheme.theme.windowMargin * 2

            onClicked: boxIsChecked = !boxIsChecked
        }

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter

            labelText: "Reactivating with this email may not restore your saved information"
            leftMargin: 0
            isChecked: boxIsChecked
            showDivider: false
            width: parent.width - VPNTheme.theme.windowMargin * 2

            onClicked: boxIsChecked = !boxIsChecked
        }

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter

            labelText: "Any extensions and themes that you’ve published to addons.mozilla.org will be deleted"
            leftMargin: 0
            isChecked: boxIsChecked
            showDivider: false
            width: parent.width - VPNTheme.theme.windowMargin * 2

            onClicked: boxIsChecked = !boxIsChecked
        }
    }

    _footerContent: ColumnLayout {
        Layout.alignment: Qt.AlignHCenter
        spacing: VPNTheme.theme.windowMargin

        VPNButton {
            Layout.fillWidth: true

            // Delete account
            text: "Delete account"
            onClicked: {
            }
        }

        VPNLinkButton {
            Layout.fillWidth: true

            fontName: VPNTheme.theme.fontBoldFamily
            // Cancel
            labelText: VPNl18n.InAppSupportWorkflowSupportSecondaryActionText
            linkColor: VPNTheme.theme.redButton
            onClicked: {
            }
        }

    }
}
