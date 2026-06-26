/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

ColumnLayout {
    id: root

    property int dividerSpacing: MZTheme.theme.toggleRowDividerSpacing


    spacing: dividerSpacing

    MZToggleRow {
            objectName: "enableAntiCensorshipFeatures"
            id: antiCensorshipFeaturesToggle

            labelText: MZI18n.SettingsAntiCensorshipSettingsEnable
            checked: MZSettings.antiCensorshipPolicy != MZSettings.NoAntiCensorship
            dividerTopMargin: dividerSpacing
            onClicked: {
                MZSettings.antiCensorshipPolicy = antiCensorshipFeaturesToggle.checked ? MZSettings.NoAntiCensorship : MZSettings.lastAntiCensorshipPolicy;
            }
    }


    Repeater {
        id: repeater

        model: [
            {
                objectName: "alwaysUsePort53",
                visible: true,
                settingValue: MZSettings.Port53,
                settingTitle: MZI18n.SettingsAntiCensorshipPort53Title,
                settingDescription: MZI18n.SettingsAntiCensorshipPort53Body,
            }, {
                objectName: "udpOverTcp",
                visible: MZFeatureList.get("obfuscationUdpOverTcp").isSupported,
                settingValue: MZSettings.UdpOverTcp,
                settingTitle: MZI18n.SettingsAntiCensorshipUdpOverTcpTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipUdpOverTcpBody,
            }, {
                objectName: "lwo",
                visible: MZFeatureList.get("obfuscationLwo").isSupported,
                settingValue: MZSettings.LWO,
                settingTitle: MZI18n.SettingsAntiCensorshipLwoTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipLwoBody,
            }, {
                objectName: "lwoOverPort53",
                visible: MZFeatureList.get("obfuscationLwo").isSupported,
                settingValue: MZSettings.LwoOverPort53,
                settingTitle: MZI18n.SettingsAntiCensorshipLwoOverPort53Title,
                settingDescription: MZI18n.SettingsAntiCensorshipLwoOverPort53Body,
            }, {
                objectName: "masque",
                visible: MZFeatureList.get("obfuscationMasque").isSupported,
                settingValue: MZSettings.Masque,
                settingTitle: MZI18n.SettingsAntiCensorshipMasqueTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipMasqueBody,
            },{
                objectName: "shadowsocks",
                visible: MZFeatureList.get("obfuscationShadowsocks").isSupported,
                settingValue: MZSettings.Shadowsocks,
                settingTitle: MZI18n.SettingsAntiCensorshipShadowsocksTitle,
                settingDescription: MZI18n.SettingsAntiCensorshipShadowsocksBody,
            }
        ];

        delegate: RowLayout {
            spacing: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin / 2
            visible: modelData.visible
            enabled: antiCensorshipFeaturesToggle.checked

            MZRadioButton {
                objectName: modelData.objectName

                Layout.preferredWidth: MZTheme.theme.vSpacing
                Layout.preferredHeight: MZTheme.theme.rowHeight
                Layout.alignment: Qt.AlignTop
                checked: MZSettings.antiCensorshipPolicy == modelData.settingValue || MZSettings.lastAntiCensorshipPolicy == modelData.settingValue
                accessibleName: `${modelData.settingTitle}. ${modelData.settingDescription}`
                onClicked: () => {
                    MZSettings.antiCensorshipPolicy = modelData.settingValue;
                    MZSettings.lastAntiCensorshipPolicy = modelData.settingValue;
                }
            }

            ColumnLayout {
                spacing: 4

                MZInterLabel {
                    Layout.fillWidth: true

                    color: MZTheme.colors.fontColorDark
                    text: modelData.settingTitle
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft
                }

                MZTextBlock {
                    text: modelData.settingDescription
                    Layout.fillWidth: true
                }
            }

            MZMouseArea {
                anchors.fill: parent

                width: Math.min(parent.implicitWidth, parent.width)
                propagateClickToParent: false
                onClicked: () => {
                    MZSettings.antiCensorshipPolicy = modelData.settingValue;
                    MZSettings.lastAntiCensorshipPolicy = modelData.settingValue;
                }
            }
        }
    }
}
