/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import QtQuick 2.5
import "../../themes/themes.js" as Theme
import "../../themes/colors.js" as Color
import "../../components"

Rectangle {
    property bool showInteractionStates: true
    property bool showError: false
    property variant itemToFocus: parent

    id: bg

    anchors.fill: parent
    antialiasing: true
    border.color: itemToFocus.activeFocus && showInteractionStates ? showError ? Color.input.error.border : Color.input.focus.border : Color.input.default.border
    border.width: itemToFocus.activeFocus && showInteractionStates ? 2 : 1
    color: Theme.input.backgroundColor
    radius: Theme.cornerRadius

    Behavior on border.color {
        ColorAnimation {
            duration: 100
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: border.width * -1
        antialiasing: true
        border.color: showError ? Color.input.error.highlight : Color.input.focus.highlight
        border.width: 4
        color: "transparent"
        opacity: itemToFocus.activeFocus && showInteractionStates ? 1 : 0
        radius: parent.radius + anchors.margins * -1
        z: -1

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }
}
