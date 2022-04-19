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
    _subtitleText: VPNl18n.SubscriptionManagementDeleteAccountSubheadline.arg(VPNAuthInApp.emailAddress)

    _inputs: ColumnLayout {
        spacing: VPNTheme.theme.listSpacing

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter
            labelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelOne
            leftMargin: 0
            isChecked: boxIsChecked
            showDivider: false
            width: parent.width - VPNTheme.theme.windowMargin * 2

            onClicked: boxIsChecked = !boxIsChecked
        }

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter

            labelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelTwo
            leftMargin: 0
            isChecked: boxIsChecked
            showDivider: false
            width: parent.width - VPNTheme.theme.windowMargin * 2

            onClicked: boxIsChecked = !boxIsChecked
        }

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter

            labelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelThree
            leftMargin: 0
            isChecked: boxIsChecked
            showDivider: false
            width: parent.width - VPNTheme.theme.windowMargin * 2

            onClicked: boxIsChecked = !boxIsChecked
        }

        VPNCheckBoxRow {
            property bool boxIsChecked: false

            Layout.alignment: Qt.AlignHCenter

            labelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelFour
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
            text: VPNl18n.SubscriptionManagementDeleteAccountButton
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
