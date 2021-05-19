/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme
import "/glean/load.js" as Glean


VPNFlickable {
    id: vpnFlickable
    property alias isSettingsView: menu.isSettingsView
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff) ||
        (VPNController.state === VPNController.StateInitializing)

    VPNMenu {
        id: menu
        title: qsTrId("Developer Options")
        isSettingsView: false
    }

    VPNCheckBoxRow {
        id: developerUnlock
        objectName: "settingsDeveloperUnlock"

        anchors.top: menu.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        labelText: qsTrId("Show Developer Options")
        subLabelText: qsTrId("Show or hide the developer options menu")

        isChecked: VPNSettings.developerUnlock
        onClicked: VPNSettings.developerUnlock = !VPNSettings.developerUnlock
    }

    VPNCheckBoxRow {
        id: stagingServer
        objectName: "settingsStagingServer"

        anchors.top: developerUnlock.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        labelText: qsTrId("Use Staging Servers")
        subLabelText: qsTrId("Staging services are used to test features before they go live to production")

        isChecked: (VPNSettings.stagingServer)
        isEnabled: vpnFlickable.vpnIsOff
        onClicked: {
            if (vpnFlickable.vpnIsOff) {
                VPNSettings.stagingServer = !VPNSettings.stagingServer
            }
       }
    }

    VPNCheckBoxRow {
        id: untestedFeatures
        objectName: "settingsUntestedFeatures"

        anchors.top: stagingServer.bottom
        anchors.topMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin

        labelText: qsTrId("Enable Untested Features")
        subLabelText: qsTrId("Enable features that may be incomplete or buggy")

        isChecked: (VPNSettings.untestedFeatures)
        isEnabled: vpnFlickable.vpnIsOff
        onClicked: {
            if (vpnFlickable.vpnIsOff) {
                VPNSettings.untestedFeatures = !VPNSettings.untestedFeatures
            }
       }
    }
}
