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
    _imgSource: "qrc:/nebula/resources/avatar-delete-account.svg"

    _inputs: ColumnLayout {
        VPNTextBlock {
            color: VPNTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft
            text: VPNl18n.SubscriptionManagementDeleteAccountSubheadline
                .arg("<b style='color: "
                    + VPNTheme.theme.fontColorDark
                    + ";'>test@mozilla.com" + VPNAuthInApp.emailAddress
                    + "</b>")
            textFormat: Text.RichText
            width: parent.width - VPNTheme.theme.windowMargin * 2

            Layout.fillWidth: true
            Layout.bottomMargin: VPNTheme.theme.vSpacing
        }

        VPNCheckBoxRow {
            property bool _isChecked: false
            id: optionOne

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            labelText: "Paid subscriptions"
            subLabelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelOne
            leftMargin: 0
            isChecked: _isChecked
            showDivider: false

            onClicked: _isChecked = !_isChecked
        }

        VPNCheckBoxRow {
            property bool _isChecked: false
            id: optionTwo

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            labelText: "Saved information"
            subLabelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelTwo
            leftMargin: 0
            isChecked: _isChecked
            showDivider: false

            onClicked: _isChecked = !_isChecked
        }

        VPNCheckBoxRow {
            property bool _isChecked: false
            id: optionThree

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            labelText: "Reactivation"
            subLabelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelThree
            leftMargin: 0
            isChecked: _isChecked
            showDivider: false

            onClicked: _isChecked = !_isChecked
        }

        VPNCheckBoxRow {
            property bool _isChecked: false
            id: optionFour

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            labelText: "Extension and themes"
            subLabelText: VPNl18n.SubscriptionManagementDeleteAccountOptionLabelFour
            leftMargin: 0
            isChecked: _isChecked
            showDivider: false

            onClicked: _isChecked = !_isChecked
        }
    }

    _disclaimers: RowLayout {
        spacing: VPNTheme.theme.vSpacing / 2

        VPNIcon {
            source: "qrc:/ui/resources/connection-info-dark.svg"
            sourceSize.width: VPNTheme.theme.iconSize
            sourceSize.height: VPNTheme.theme.iconSize

            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        }

        VPNTextBlock {
            id: text
            text: "In order to delete your account we need your consent on the options above."
            Layout.fillWidth: true
        }
    }

    _footerContent: ColumnLayout {
        Layout.alignment: Qt.AlignHCenter

        spacing: VPNTheme.theme.vSpacing

        VPNButton {
            Layout.fillWidth: true

            colorScheme: VPNTheme.theme.redButton
            enabled: optionOne.isChecked && optionTwo.isChecked && optionThree.isChecked && optionFour.isChecked
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
