/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme


RadioButton {
    property var value
    property alias iconSource: img.source

    id: radio
    checked: false
    implicitHeight: 30
    implicitWidth: 30
    activeFocusOnTab: true

    indicator: VPNIcon {
        id: img
        anchors.centerIn: parent
        antialiasing: true
        sourceSize.height: 30
        sourceSize.width: 30

    }

    ColorOverlay {
        anchors.fill: img
        source: img
        color: radio.checked ? Theme.blue : Theme.fontColor
        Behavior on color {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    VPNInputBackground {
        radius: 22
        border.color: Theme.input.focusBorder
        color: "transparent"
        border.width: 2
        anchors.margins: -2
        z: -1
        opacity: radio.activeFocus ? 1 : 0
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }
}
