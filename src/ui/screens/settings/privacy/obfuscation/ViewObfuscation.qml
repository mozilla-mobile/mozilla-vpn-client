/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: root
    objectName: "obfuscationSettingsView"

    property Component rightMenuButton: Component {
        Loader {
            active: true
            sourceComponent: MZIconButton {
                objectName: "obfuscationHelpButton"

                onClicked: {
                    helpSheet.open()
                }

                accessibleName: MZI18n.GetHelpLinkText

                Image {
                    anchors.centerIn: parent

                    source: MZAssetLookup.getImageSource("Question")
                    fillMode: Image.PreserveAspectFit
                }
            }
        }
    }

    _menuTitle: MZI18n.SettingsObfuscationSettings

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin * 1.5
        Layout.preferredWidth: parent.width

        MZInformationCard {
            objectName: "obfuscationSettingsViewInformationCard"
            cardType: MZInformationCard.CardType.Warning
            Layout.preferredWidth: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
            Layout.alignment: Qt.AlignHCenter

            _infoContent: ColumnLayout {
                id: textBlocks
                spacing: 5

                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.SettingsObfuscationSettingsWarning1
                    verticalAlignment: Text.AlignVCenter
                }

                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.SettingsObfuscationSettingsWarning2
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        ObfuscationFeaturesList {
            id: obfuscationFeaturesList
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.fillWidth: true
        }
    }

    MZHelpSheet {
        id: helpSheet
        objectName: "obfuscationHelpSheet"

        title: MZI18n.HelpSheetsObfuscationTitle

        model: [
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsObfuscationBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsObfuscationBody2, margin: MZTheme.theme.helpSheetBodySpacing}
        ]
    }
}
