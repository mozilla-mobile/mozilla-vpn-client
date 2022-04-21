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

    property var checkboxData: [
        {
            labelText: "Paid subscriptions",
            subLabelText: VPNl18n.DeleteAccountOptionLabelOne,
            isSelected: false
        },
        {
            labelText: "Saved information",
            subLabelText: VPNl18n.DeleteAccountOptionLabelTwo,
            isSelected: false
        },
        {
            labelText: "Reactivation",
            subLabelText: VPNl18n.DeleteAccountOptionLabelThree,
            isSelected: false
        },
        {
            labelText: "Extensions and themes",
            subLabelText: VPNl18n.DeleteAccountOptionLabelFour,
            isSelected: false
        }
    ]
    property bool allowAccountDeletion: false

    _changeEmailLinkVisible: false
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        viewDeleteAccount.cancelAccountDeletion();
    }
    _headlineText: VPNl18n.DeleteAccountHeadline
    _imgSource: "qrc:/nebula/resources/avatar-delete-account.svg"

    _inputs: ColumnLayout {
        VPNTextBlock {
            color: VPNTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft
            text: VPNl18n.DeleteAccountSubheadline
                .arg("<b style='color:"
                    + VPNTheme.theme.fontColorDark
                    + ";'>test@mozilla.com"
                    + "</b>")
            textFormat: Text.RichText

            Layout.fillWidth: true
            Layout.bottomMargin: VPNTheme.theme.vSpacing
        }

        Repeater {
            model: checkboxData
            delegate: VPNCheckBoxRow {
                labelText: modelData.labelText
                subLabelText: modelData.subLabelText
                leftMargin: 0
                isChecked: modelData.isSelected
                showDivider: false

                onClicked: isChecked = !isChecked
                onIsCheckedChanged: handleCheckboxChange(index, isChecked)
            }
        }
    }

    _disclaimers: RowLayout {
        Layout.alignment: Qt.AlignHCenter
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
            enabled: viewDeleteAccount.allowAccountDeletion
            // Delete account
            text: VPNl18n.DeleteAccountButtonLabel
            onClicked: if (viewDeleteAccount.allowAccountDeletion) {
                VPNAuthInApp.deleteAccount();
            }
        }

        VPNLinkButton {
            Layout.fillWidth: true

            fontName: VPNTheme.theme.fontBoldFamily
            // Cancel
            labelText: VPNl18n.InAppSupportWorkflowSupportSecondaryActionText
            linkColor: VPNTheme.theme.redButton
            onClicked: {
                viewDeleteAccount.cancelAccountDeletion();
            }
        }
    }

    function handleCheckboxChange(checkboxIndex, isChecked) {
        checkboxData[checkboxIndex].isSelected = isChecked;
        viewDeleteAccount.allowAccountDeletion = checkboxData.every(checkbox =>
            checkbox.isSelected);
    }

    function cancelAccountDeletion() {
        VPN.cancelAccountDeletion();
        VPN.cancelAuthentication();
    }
}
