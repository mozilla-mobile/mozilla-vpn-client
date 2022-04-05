/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import components 0.1

Rectangle {
    property variant itemToFocus: parent

    id: inputBorder

    anchors.fill: parent
    antialiasing: true
    border.color: VPNTheme.colors.input.default.border
    radius: VPNTheme.theme.cornerRadius

    Behavior on border.color {
        ColorAnimation {
            duration: 50
        }
    }

    Rectangle {
        id: highlight

        anchors.fill: parent
        anchors.margins: border.width * -1
        antialiasing: true
        border.color: itemToFocus && itemToFocus.hasError ? VPNTheme.colors.input.error.highlight : VPNTheme.colors.input.focus.highlight
        border.width: 4
        color: VPNTheme.theme.transparent
        opacity: itemToFocus && itemToFocus.activeFocus && itemToFocus.showInteractionStates ? 1 : 0
        radius: parent.radius + anchors.margins * -1
        z: -1

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }
}
