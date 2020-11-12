/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// IMPORTANT: this file is used only for mobile builds.

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    id: logs

    VPNMenu {
        id: menu
        isMainView: true

        //% "View Logs"
        title: qsTrId("vpn.viewlogs.title")
    }

    ScrollView {
        id: logScrollView

        height: parent.height - menu.height - copyLogs.height
        width: parent.width
        anchors.top: menu.bottom

        VPNTextBlock {
            id: logText

            text: VPN.retrieveLogs()
        }
    }

    VPNButton {
        id: copyLogs

        width: parent.width
        anchors.top: logScrollView.bottom

        //% "Copy"
        //: Copy into the OS clipboard
        text: qsTrId("vpn.viewLogs.copy")
        radius: 4
        onClicked: VPN.storeInClipboard(logText.text)
    }
}
