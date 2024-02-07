/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import compat 0.1
import "qrc:/ui/sharedViews"

ViewFullScreen {
    property string _menuTitle: MZI18n.ResetSettingsResetLabel

    content: ColumnLayout {
        spacing : 0

        Image {
            id: icon

            Layout.alignment: Qt.AlignHCenter

            sourceSize.height: 80
            sourceSize.width: 80
            fillMode: Image.PreserveAspectFit
            source: "qrc:/ui/resources/reset.svg"
        }

        MZHeadline {
            Layout.topMargin: 24
            Layout.fillWidth: true

            text: MZI18n.ResetSettingsTitle
        }

        MZInterLabel {
            Layout.topMargin: 8
            Layout.fillWidth: true

            text: MZI18n.ResetSettingsBody1
            color: MZTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft
        }

        MZInterLabel {
            Layout.topMargin: 16
            Layout.fillWidth: true

            text: "<ul style='margin-left: -20px;'><li>%1</li></ul>".arg(MZI18n.ResetSettingsListItem1)
            textFormat: Text.RichText
            color: MZTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft

            Accessible.name: MZI18n.ResetSettingsListItem1
        }

        MZInterLabel {
            Layout.topMargin: 12
            Layout.fillWidth: true

            text: "<ul style='margin-left: -20px;'><li>%1</li></ul>".arg(MZI18n.ResetSettingsListItem2)
            textFormat: Text.RichText
            color: MZTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft

            Accessible.name: MZI18n.ResetSettingsListItem2
        }

        MZInterLabel {
            Layout.topMargin: 12
            Layout.fillWidth: true

            text: "<ul style='margin-left: -20px;'><li>%1</li></ul>".arg(MZI18n.ResetSettingsListItem3)
            textFormat: Text.RichText
            color: MZTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft

            Accessible.name: MZI18n.ResetSettingsListItem3
        }

        MZInterLabel {
            Layout.topMargin: 16
            Layout.fillWidth: true

            text: MZI18n.ResetSettingsBody2
            color: MZTheme.theme.fontColor
            horizontalAlignment: Text.AlignLeft
        }
    }


    buttons: ColumnLayout {
        spacing: 0

        MZButton {
            Layout.fillWidth: true

            colorScheme: MZTheme.theme.redButton
            text: MZI18n.ResetSettingsResetButtonLabel

            onClicked: confirmResetLoader.active = true
        }

        MZLinkButton {
            objectName: "privacyBackButton"

            Layout.topMargin: 16
            Layout.fillWidth: true

            implicitHeight: MZTheme.theme.rowHeight
            labelText: MZI18n.GlobalGoBack

            onClicked: getHelpStackView.pop()
        }
    }

    Loader {
        id: confirmResetLoader

        active: false
        sourceComponent: MZSimplePopup {
            id: confirmResetPopup

            anchors.centerIn: Overlay.overlay
            imageSrc: "qrc:/ui/resources/confirm-reset.svg"
            imageSize: Qt.size(80, 80)
            title: MZI18n.ResetSettingsConfirmResetModalTitle
            description: MZI18n.ResetSettingsConfirmResetModalBody
            buttons: [
                MZButton {
                    Layout.fillWidth: true

                    text: MZI18n.ResetSettingsConfirmResetModalResetButtonLabel
                    colorScheme: MZTheme.theme.redButton

                    onClicked: confirmResetPopup.close()
                },
                MZLinkButton {
                    Layout.alignment: Qt.AlignHCenter

                    labelText: MZI18n.InAppSupportWorkflowSupportSecondaryActionText

                    onClicked: VPN.hardResetAndQuit()
                }
            ]

            onClosed: {
                confirmResetLoader.active = false
            }
        }

        onActiveChanged: if (active) { item.open() }
    }
}
