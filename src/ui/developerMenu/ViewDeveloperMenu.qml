/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    property string _menuTitle: VPNl18n.tr(VPNl18n.SettingsDevTitle)
    property alias isSettingsView: menu.isSettingsView
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff) ||
        (VPNController.state === VPNController.StateInitializing)

    id: vpnFlickable
    VPNMenu {
        id: menu
        title: VPNl18n.tr(VPNl18n.SettingsDevTitle)
        isSettingsView: false
    }

    VPNCheckBoxRow {
        id: developerUnlock

        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        labelText:  VPNl18n.tr(VPNl18n.SettingsDevShowOptionTitle)
        subLabelText: VPNl18n.tr(VPNl18n.SettingsDevShowOptionSubtitle)

        isChecked: VPNSettings.developerUnlock
        onClicked: VPNSettings.developerUnlock = !VPNSettings.developerUnlock
    }

    VPNCheckBoxRow {
        id: stagingServer
        objectName: "settingsStagingServer"

        anchors.top: developerUnlock.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        labelText: VPNl18n.tr(VPNl18n.SettingsDevUseStagingTitle)
        subLabelText: VPNl18n.tr(VPNl18n.SettingsDevUseStagingSubtitle)

        isChecked: (VPNSettings.stagingServer)
        isEnabled: vpnFlickable.vpnIsOff
        onClicked: {
            if (vpnFlickable.vpnIsOff) {
                VPNSettings.stagingServer = !VPNSettings.stagingServer
            }
        }
    }

    VPNSettingsItem {
        id: featureListLink
        objectName: "settingsFeatureList"

        anchors.top: stagingServer.bottom
        anchors.topMargin: Theme.windowMargin
        anchors.left: stagingServer.left
        anchors.leftMargin: Theme.windowMargin/2
        width: parent.width - Theme.windowMargin

        // Do not translate this string!
        settingTitle: "Feature list"
        imageLeftSrc: "../resources/settings/whatsnew.svg"
        imageRightSrc: "../resources/chevron.svg"
        onClicked: stackview.push("../developerMenu/ViewFeatureList.qml")
    }

    VPNCheckBoxAlert {
        anchors.top: featureListLink.bottom
        visible: VPN.stagingMode != VPNSettings.stagingServer

        errorMessage: VPNl18n.tr(VPNl18n.SettingsDevRestartRequired)
    }
}
