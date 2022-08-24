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
    objectName: "advancedDNSSettingsView"

    _menuTitle: VPNl18n.CustomDNSSettingsDnsNavItem
    _interactive: false

    _viewContentData: VPNTabNavigation {
        id: tabs
        Layout.topMargin: -VPNTheme.theme.windowMargin
        Layout.preferredHeight: root.height - VPNTheme.theme.menuHeight
        Layout.preferredWidth: root.width

        tabList: ListModel {
            id: tabButtonList
            ListElement {
                tabLabelStringId: "CustomDNSSettingsDnsDefaultToggle"
                tabButtonId: "tabDefault"
            }
            ListElement {
                tabLabelStringId:"CustomDNSSettingsDnsAdvancedToggle"
                tabButtonId: "tabAdvanced"
            }
        }

        ButtonGroup {
            id: radioButtonGroup
        }

        stackContent: [
            DNSSettingsTabContent {
                settingsListModel: ListModel {
                    id:defaultTabListModel
                }
                Component.onCompleted: {
                    defaultTabListModel.append({
                                                settingValue: VPNSettings.Gateway,
                                                settingTitle: VPNl18n.CustomDNSSettingsDnsDefaultRadioHeader,
                                                settingDescription: VPNl18n.CustomDNSSettingsDnsDefaultRadioBody,
                                                showDNSInput: false,
                    })
                }
            },
            DNSSettingsTabContent {
                settingsListModel: ListModel{
                    id:advancedListModel
                }
                // hacks to circumvent the fact that we can't send
                // "scripts" as property values through ListModel/ListElement
                Component.onCompleted: {
                    advancedListModel.append({
                                                 settingValue: VPNSettings.BlockAds,
                                                 settingTitle: VPNl18n.CustomDNSSettingsDnsAdblockRadioHeader,
                                                 settingDescription: VPNl18n.CustomDNSSettingsDnsAdblockRadioBody,
                                                 showDNSInput: false})
                    advancedListModel.append({   settingValue: VPNSettings.BlockTracking,
                                                 settingTitle: VPNl18n.CustomDNSSettingsDnsAntitrackRadioHeader,
                                                 settingDescription: VPNl18n.CustomDNSSettingsDnsAntitrackRadioBody,
                                                 showDNSInput: false})
                    advancedListModel.append({   settingValue: VPNSettings.BlockAll,
                                                 settingTitle: VPNl18n.CustomDNSSettingsDnsAdblockAntiTrackRadioHeader,
                                                 settingDescription: VPNl18n.CustomDNSSettingsDnsAdblockAntiTrackRadioBody,
                                                 showDNSInput: false})
                    advancedListModel.append({   settingValue: VPNSettings.Custom,
                                                 settingTitle: VPNl18n.CustomDNSSettingsDnsCustomDNSRadioHeader,
                                                 settingDescription:  VPNl18n.CustomDNSSettingsDnsCustomDNSRadioBody,
                                                 showDNSInput: true})
                }
            }

        ]
        Component.onCompleted: {
            if (VPNSettings.dnsProvider !== VPNSettings.Gateway) {
                return tabs.setCurrentTabIndex(1)
            }
        }
    }
}

