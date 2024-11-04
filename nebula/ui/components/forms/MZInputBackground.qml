/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import components 0.1

Rectangle {
    property variant itemToFocus: parent

    id: inputBorder

    anchors.fill: parent
    antialiasing: true
    border.color: MZTheme.colors.inputState.default.border
    radius: MZTheme.theme.cornerRadius

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
        border.color: itemToFocus && itemToFocus.hasError ? MZTheme.colors.inputState.error.highlight : MZTheme.colors.inputState.focus.highlight
        border.width: 4
        color: MZTheme.colors.transparent
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
