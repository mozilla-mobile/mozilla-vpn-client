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
import compat 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: root
    objectName: "antiCensorshipSettingsView"

    property Component rightMenuButton: Component {
        Loader {
            active: true
            sourceComponent: MZIconButton {
                objectName: "antiCensorshipHelpButton"

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

    _menuTitle: MZI18n.SettingsAntiCensorshipSettings

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin * 1.5
        Layout.preferredWidth: parent.width

        MZInformationCard {
            objectName: "antiCensorshipSettingsViewInformationCard"
            cardType: MZInformationCard.CardType.Warning
            Layout.preferredWidth: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
            Layout.alignment: Qt.AlignHCenter

            _infoContent: ColumnLayout {
                id: textBlocks
                spacing: 0

                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.SettingsAntiCensorshipSettingsWarning1
                    verticalAlignment: Text.AlignVCenter
                }

                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.SettingsAntiCensorshipSettingsWarning2
                    verticalAlignment: Text.AlignVCenter
                }

                Loader {
                    active: !VPNController.silentServerSwitchingSupported && VPNController.state !== VPNController.StateOff
                    Layout.fillWidth: true
                    visible: active
                    sourceComponent: MZTextBlock {
                        width: parent.width
                        text: MZI18n.SettingsDnsSettingsDisconnectWarning
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        AntiCensorshipFeaturesList {
            id: antiCensorshipFeaturesList
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.fillWidth: true
        }
    }

    MZHelpSheet {
        id: helpSheet
        objectName: "antiCensorshipHelpSheet"

        title: MZI18n.HelpSheetsAntiCensorshipTitle

        model: [
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsAntiCensorshipBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsAntiCensorshipBody2, margin: MZTheme.theme.helpSheetBodySpacing}
        ]
    }
}
