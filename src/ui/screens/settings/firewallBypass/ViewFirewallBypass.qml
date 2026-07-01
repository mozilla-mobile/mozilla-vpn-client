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
    objectName: "firewallBypassSettingsView"

    property Component rightMenuButton: Component {
        Loader {
            active: true
            sourceComponent: MZIconButton {
                objectName: "firewallBypassHelpButton"

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

    _menuTitle: MZI18n.SettingsFirewallBypassSettings

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin * 1.5
        Layout.preferredWidth: parent.width

        MZInformationCard {
            objectName: "firewallBypassSettingsViewInformationCard"
            cardType: MZInformationCard.CardType.Warning
            Layout.preferredWidth: Math.min(window.width - MZTheme.theme.windowMargin * 2, MZTheme.theme.navBarMaxWidth)
            Layout.alignment: Qt.AlignHCenter

            _infoContent: ColumnLayout {
                id: textBlocks
                spacing: 0

                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.SettingsFirewallBypassSettingsWarning1
                    verticalAlignment: Text.AlignVCenter
                }

                MZTextBlock {
                    Layout.fillWidth: true
                    width: undefined
                    text: MZI18n.SettingsFirewallBypassSettingsWarning2
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

        FirewallBypassFeaturesList {
            id: firewallBypassFeaturesList
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.fillWidth: true
        }
    }

    MZHelpSheet {
        id: helpSheet
        objectName: "firewallBypassHelpSheet"

        title: MZI18n.HelpSheetsFirewallBypassTitle

        model: [
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsFirewallBypassBody1, margin: MZTheme.theme.helpSheetTitleBodySpacing},
            {type: MZHelpSheet.BlockType.Text, text: MZI18n.HelpSheetsFirewallBypassBody2, margin: MZTheme.theme.helpSheetBodySpacing}
        ]
    }
}
