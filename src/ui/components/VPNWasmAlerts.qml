/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import "../themes/themes.js" as Theme

Item {
    anchors.fill: parent

    Column {
        anchors.fill: parent
        anchors.top: parent.top
        anchors.topMargin: Theme.rowHeight
        spacing: Theme.windowMargin

        VPNAlert {
            id: alertBox1
            alertType: "update"
            state: "recommended"
            alertColor: Theme.blueButton
            width: parent.width - (Theme.windowMargin * 2)
            alertLinkText: qsTrId("vpn.updates.updateNow")
            alertText: qsTrId("vpn.updates.newVersionAvailable")
            visible: true
        }

        Repeater {
            model: 7
            delegate: VPNSystemAlert {
                state: index
                visible: true
            }
        }
    }
}
