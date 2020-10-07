/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

// VPNClickableRow
Control {
    // TODO: We can set the criteria on a row by row basis for
    // when that row should be disabled
    property var rowShouldBeDisabled: false

    property var backgroundColor: Theme.greyButton
    signal clicked

    id: mainRow
    height: 40
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: Theme.windowMargin / 2
    anchors.rightMargin: Theme.windowMargin / 2

    width: parent.width - (Theme.windowMargin * 2)
    opacity: rowShouldBeDisabled ? .7 : 1

    background: Rectangle {
        id: rowBackground
        anchors.fill: mainRow
        radius: 4
        color: {
            if (rowShouldBeDisabled && mouseArea.pressed) {
                return Theme.bgColor
            }

            if (mouseArea.pressed) {
                return backgroundColor.buttonPressed
            }

            if (mouseArea.containsMouse) {
                return backgroundColor.buttonHovered
            }

            // The default color is set to Theme.bgColor
            // because transitioning from "transparent" to
            // other hex values results in weirdness.
            return Theme.bgColor
        }

        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
        Behavior on opacity {
            PropertyAnimation {
                duration: 200
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        cursorShape: rowShouldBeDisabled ? Qt.ForbiddenCursor : Qt.PointingHandCursor
        hoverEnabled: !rowShouldBeDisabled
        focus: !rowShouldBeDisabled
        onClicked: {
            if (rowShouldBeDisabled) {
                return
            }
            return mainRow.clicked()
        }
    }

    transitions: [
        Transition {
            NumberAnimation {
                target: mainRow
                property: "opacity"
                duration: 200
            }
        }
    ]
}
