/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


VPNViewBase {
    id: root
    objectName: "privacySettingsView"

    _menuTitle: VPNl18n.SettingsPrivacySettings
    _interactive: false

    _viewContentData: Column {
        id: tabs
        Layout.topMargin: -VPNTheme.theme.windowMargin
        Layout.preferredHeight: root.height - VPNTheme.theme.menuHeight
        Layout.preferredWidth: root.width

        Repeater {
            id: repeater

            delegate: RowLayout {
                Layout.fillWidth: true
                spacing: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin

                VPNRadioButton {
                    id: radioButton
                    Layout.preferredWidth: VPNTheme.theme.vSpacing
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.alignment: Qt.AlignTop
                    checked: VPNSettings.dnsProvider == modelData.settingValue
                    accessibleName: modelData.settingTitle
                    onClicked: VPNSettings.dnsProvider = modelData.settingValue
                }

                ColumnLayout {
                    spacing: 4
                    Layout.fillWidth: true

                    VPNInterLabel {
                        Layout.fillWidth: true

                        text: modelData.settingTitle
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignLeft

                        VPNMouseArea {
                            anchors.fill: undefined
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom

                            enabled: radioButton.enabled
                            width: Math.min(parent.implicitWidth, parent.width)
                            propagateClickToParent: false
                            onClicked: VPNSettings.dnsProvider = modelData.settingValue
                        }
                    }

                    VPNTextBlock {
                        text: modelData.settingDescription

                        Layout.fillWidth: true
                    }
                }
            }
        }

        Component.onCompleted: {
                repeater.model = [{
                                     settingValue: VPNSettings.BlockAds,
                                     settingTitle: VPNl18n.CustomDNSSettingsDnsAdblockRadioHeader,
                                     settingDescription: VPNl18n.CustomDNSSettingsDnsAdblockRadioBody,
                                    },
         {   settingValue: VPNSettings.BlockTracking,
                                             settingTitle: VPNl18n.CustomDNSSettingsDnsAntitrackRadioHeader,
                                             settingDescription: VPNl18n.CustomDNSSettingsDnsAntitrackRadioBody,
                                             },
          {   settingValue: VPNSettings.BlockAll,
                                             settingTitle: VPNl18n.CustomDNSSettingsDnsAdblockAntiTrackRadioHeader,
                                             settingDescription: VPNl18n.CustomDNSSettingsDnsAdblockAntiTrackRadioBody,
                                             }];
        }
    }
}

