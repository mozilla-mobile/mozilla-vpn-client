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
    objectName: "viewDeleteAccountRequest"

    property var checkboxData: [
        {
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionOne,
            objectName: "check1",
            isSelected: false
        },
        {
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionTwo,
            objectName: "check2",
            isSelected: false
        },
        {
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionThree,
            objectName: "check3",
            isSelected: false
        },
        {
            subLabelText: VPNl18n.DeleteAccountOptionDescriptionFour,
            objectName: "check4",
            isSelected: false
        }
    ]
    property bool allowAccountDeletion: false

    _changeEmailLinkVisible: false
    _viewObjectName: "authDeleteAccountRequest"
    _menuButtonAccessibleName: qsTrId("vpn.main.back")
    _menuButtonImageSource: "qrc:/nebula/resources/back.svg"
    _menuButtonOnClick: () => {
        cancelAuthenticationFlow();
    }
    _headlineText: VPNl18n.DeleteAccountHeadline
    _imgSource: "qrc:/nebula/resources/avatar-delete-account.svg"

    _inputs: ColumnLayout {
        objectName: "accountDeletionLayout"
        VPNTextBlock {
            objectName: "accountDeletionLabel"
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
                objectName: "accountDeletionCheckbox-" + modelData.objectName;
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
            objectName: "deleteAccountForRealButton"

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
                cancelAuthenticationFlow();
            }
        }
    }

    function handleCheckboxChange(checkboxIndex, isChecked) {
        checkboxData[checkboxIndex].isSelected = isChecked;
        viewDeleteAccount.allowAccountDeletion = checkboxData.every(checkbox =>
            checkbox.isSelected);
    }
}
