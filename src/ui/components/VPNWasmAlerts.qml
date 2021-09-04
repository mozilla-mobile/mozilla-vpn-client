/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import "../themes/themes.js" as Theme

VPNFlickable {
    id: vpnFlickable
    anchors.fill: parent
    contentHeight: wasmAlerts.implicitHeight

    Column {
        id: wasmAlerts
        anchors.top: parent.top
        anchors.topMargin: 56 + Theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Theme.windowMargin
        width: parent.width

        Repeater {
            model: 11
            delegate: VPNSystemAlert {
                wasmSetAlertState: index
                visible: true
            }
        }

        VPNAlerts {
            width: parent.width - Theme.windowMargin
            anchors.horizontalCenter: parent.horizontalCenter
            isWasmViewer: true
        }
    }
}
