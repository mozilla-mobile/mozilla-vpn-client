/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.12
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme


Button {
    id: wasmHeader

    height: Theme.menuHeight
    width: parent.width
    z: 2
    clip: true
    onClicked: mainStackView.replace("../components/VPNWasmMenu.qml", StackView.Immediate)
    background: Rectangle {
        color: Theme.bgColor
        anchors.fill: parent
    }

    VPNUIStates {
        colorScheme: Theme.iconButtonLightBackground
        z: 2
        radius: 0
    }

    VPNMouseArea {
        hoverEnabled: true
    }

    Text {
        id: btnText

        font.pixelSize: 16
        text: "Viewer Menu"
        font.family: Theme.fontBoldFamily
        color: Theme.fontColorDark
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 3
    }

    Rectangle {
        color: "#eee"
        z: 2
        width: wasmHeader.width
        height: 1
        anchors.bottom: wasmHeader.bottom
    }
}
