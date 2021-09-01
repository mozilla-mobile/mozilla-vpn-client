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
    id: vpnFlickable
    property alias isSettingsView: menu.isSettingsView
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff) ||
        (VPNController.state === VPNController.StateInitializing)

    VPNMenu {
        id: menu
        title: VPNl18n.tr(VPNl18n.SettingsDevTitle)
        isSettingsView: false
    }

    VPNCheckBoxRow {
        id: developerUnlock
        objectName: "settingsDeveloperUnlock"

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
    VPNExternalLinkListItem {
        visible: stagingServer.isChecked && !restartRequired.visible
        anchors.top: stagingServer.bottom
        anchors.topMargin: Theme.windowMargin
        anchors.left: stagingServer.left
        anchors.leftMargin: Theme.windowMargin/2
        width: parent.width - Theme.windowMargin

        objectName: "openInspector"
        title: "Open Inspector"
        accessibleName: "Open Inspector"
        iconSource:  "../resources/externalLink.svg"
        backgroundColor: Theme.clickableRowBlue
        onClicked: {
            VPN.openLink(VPN.LinkInspector)
        }
    }

    VPNCheckBoxAlert {
        id: restartRequired
        anchors.top: stagingServer.bottom
        visible: VPN.stagingMode != VPNSettings.stagingServer

        errorMessage: VPNl18n.tr(VPNl18n.SettingsDevRestartRequired)
    }
}
