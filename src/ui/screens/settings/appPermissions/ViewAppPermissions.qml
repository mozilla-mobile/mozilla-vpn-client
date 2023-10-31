/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    //% "Search apps"
    //: Search bar placeholder text
    property string searchApps: qsTrId("vpn.protectSelectedApps.searchApps")

    //% "Add application"
    //: Button label
    property string addApplication: qsTrId("vpn.protectSelectedApps.addApplication")

    id: vpnFlickable
    objectName: "appPermissions"

    _menuTitle: MZI18n.SettingsAppExclusionSettings
    _viewContentData: ColumnLayout {

        Layout.preferredWidth: parent.width

        Loader {
            Layout.leftMargin: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.bottomMargin: 24
            Layout.fillWidth: true

            active: Qt.platform.os === "linux" && VPNController.state !== VPNController.StateOff
            visible: active

            sourceComponent: MZInformationCard {
                width: parent.width
                implicitHeight: textBlock.height + MZTheme.theme.windowMargin * 2
                _infoContent: MZTextBlock {
                    id: textBlock
                    Layout.fillWidth: true


                    text: MZI18n.SplittunnelInfoCardDescription
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        AppPermissionsList {
            id: enabledList

            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.leftMargin: MZTheme.theme.vSpacing
            Layout.rightMargin: MZTheme.theme.vSpacing

            searchBarPlaceholder: searchApps
            enabled: Qt.platform.os === "linux" ? VPNController.state === VPNController.StateOff : true
        }
    }

    Component.onCompleted: {
        console.log("Component ready");
        VPNAppPermissions.requestApplist();
        Glean.sample.appPermissionsViewOpened.record();
    }

}
