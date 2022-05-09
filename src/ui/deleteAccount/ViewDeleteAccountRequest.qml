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
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionOne,
            isSelected: false
        },
        {
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionTwo,
            isSelected: false
        },
        {
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionThree,
            isSelected: false
        },
        {
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionFour,
            isSelected: false
        }
    ]
    property bool allowAccountDeletion: false

    _changeEmailLinkVisible: false
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        cancelAccountDeletion();
    }
    _headlineText: VPNl18n.DeleteAccountHeadline
    _imgSource: "qrc:/nebula/resources/avatar-delete-account.svg"

    _inputs: ColumnLayout {
        VPNTextBlock {
            color: VPNTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft
            text: VPNl18n.DeleteAccountSubheadline
                .arg("<b style='color:" + VPNTheme.theme.fontColorDark + ";'>"
                    + VPNAuthInApp.emailAddress + "</b>")
            textFormat: Text.RichText

            Layout.fillWidth: true
            Layout.bottomMargin: VPNTheme.theme.vSpacing
        }

        Repeater {
            model: checkboxData
            delegate: VPNCheckBoxRow {
                subLabelText: modelData.subLabelText
                leftMargin: 0
                isChecked: modelData.isSelected
                showDivider: false

                onClicked: isChecked = !isChecked
                onIsCheckedChanged: handleCheckboxChange(index, isChecked)
            }
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
                cancelAccountDeletion();
            }
        }
    }

    function handleCheckboxChange(checkboxIndex, isChecked) {
        checkboxData[checkboxIndex].isSelected = isChecked;
        viewDeleteAccount.allowAccountDeletion = checkboxData.every(checkbox =>
            checkbox.isSelected);
    }
}
