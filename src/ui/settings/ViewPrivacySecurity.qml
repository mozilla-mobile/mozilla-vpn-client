/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme


Item {
    id: root
    VPNMenu {
        id: menu
        objectName: "settingsPrivacySecurtyBackButton"

        //% "Privacy & Security"
        title: qsTrId("vpn.settings.privacySecurityTitle")
        isSettingsView: true
    }
    VPNFlickable {
        id: vpnFlickable
        anchors.top: menu.bottom
        anchors.left: root.left
        anchors.right: root.right
        height: root.height - menu.height
        flickContentHeight: col.height
        interactive: flickContentHeight > height

        Column {
            id: col
            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: Theme.windowMargin

            VPNCheckBoxRow {
                id: dataCollection
                objectName: "dataCollection"
                width: parent.width - Theme.windowMargin

                //% "Data collection and use"
                labelText: qsTrId("vpn.settings.dataCollection")
                //% "Allow Mozilla VPN to send technical data to Mozilla"
                subLabelText: qsTrId("vpn.settings.dataCollection.description")
                isChecked: VPNSettings.gleanEnabled
                onClicked: {
                    VPNSettings.gleanEnabled = !VPNSettings.gleanEnabled
               }
            }
        }
    }
}
