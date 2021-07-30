/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import QtQuick 2.5
import QtQuick.Controls 2.15
import "../../themes/themes.js" as Theme
import "../../components"

Rectangle {
    property bool showInteractionStates: true
    property bool showError: false
    property variant itemToFocus: parent

    id: bg
    antialiasing: true
    border.color: itemToFocus.activeFocus && showInteractionStates || showError ? showError ?   Theme.red : Theme.input.focusBorder : Theme.input.borderColor
    border.width:  itemToFocus.activeFocus && showInteractionStates || showInteractionStates && showError ? 2 : 1
    color: Theme.input.backgroundColor
    radius: Theme.cornerRadius
    anchors.fill: parent

    Behavior on border.color {
        ColorAnimation {
            duration: 100
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: border.width * -1
        radius: parent.radius + anchors.margins*-1
        border.color: showError ? Theme.errorFocusOutline :  Theme.blueFocusOutline
        border.width: 4
        antialiasing: true
        z: -1
        opacity: itemToFocus.activeFocus && showInteractionStates ? 1 : 0
        color: "transparent"
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }
}
