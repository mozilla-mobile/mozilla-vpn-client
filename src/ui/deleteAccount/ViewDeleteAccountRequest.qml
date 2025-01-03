/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZInAppAuthenticationBase {
    id: viewDeleteAccount
    objectName: "viewDeleteAccountRequest"

    property var checkboxData: [
        {
            subLabelText: MZI18n.DeleteAccountOptionDescriptionOne,
            objectName: "check1",
            isSelected: false
        },
        {
            subLabelText: MZI18n.DeleteAccountOptionDescriptionTwo,
            objectName: "check2",
            isSelected: false
        },
        {
            subLabelText: MZI18n.DeleteAccountOptionDescriptionThree,
            objectName: "check3",
            isSelected: false
        },
        {
            subLabelText: MZI18n.DeleteAccountOptionDescriptionFour,
            objectName: "check4",
            isSelected: false
        }
    ]
    property bool allowAccountDeletion: false

    _changeEmailLinkVisible: false
    _disclaimersVisible: false
    _menuButtonAccessibleName: MZI18n.GlobalGoBack
    _menuButtonImageSource: MZAssetLookup.getImageSource("ArrowBack")
    _menuButtonImageMirror: MZLocalizer.isRightToLeft
    _menuButtonOnClick: () => {
        cancelAuthenticationFlow();
    }
    _headlineText: MZI18n.DeleteAccountHeadline2
    _imgSource: MZAssetLookup.getImageSource("AvatarDeleteAccount")

    _inputs: ColumnLayout {
        objectName: "accountDeletionLayout"
        spacing: MZTheme.theme.vSpacingSmall
        MZTextBlock {
            objectName: "accountDeletionLabel"
            color: MZTheme.colors.fontColor
            horizontalAlignment: Text.AlignLeft
            text: MZI18n.DeleteAccountSubheadline2
                .arg("<b style='color:" + MZTheme.colors.fontColorDark + ";'>"
                    + MZAuthInApp.emailAddress + "</b>")
            textFormat: Text.RichText
            font.pixelSize: MZTheme.theme.fontSize

            Layout.fillWidth: true
            Layout.bottomMargin: MZTheme.theme.listSpacing
        }

        Repeater {
            model: checkboxData
            delegate: MZCheckBoxRow {
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

        spacing: MZTheme.theme.vSpacing

        MZButton {
            Layout.fillWidth: true
            objectName: "deleteAccountForRealButton"

            colorScheme: MZTheme.colors.destructiveButton
            enabled: viewDeleteAccount.allowAccountDeletion
            // Delete account
            text: MZI18n.DeleteAccountButtonLabel
            onClicked: if (viewDeleteAccount.allowAccountDeletion) {
                MZAuthInApp.deleteAccount();
            }
        }

        MZLinkButton {
            Layout.fillWidth: true

            // Cancel
            labelText: MZI18n.InAppSupportWorkflowSupportSecondaryActionText
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
