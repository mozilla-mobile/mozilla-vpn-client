/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.6

import components.forms 0.1


Rectangle {
        // force cursor color and blink
        id: cursor
        visible: parent.activeFocus && !parent.readOnly && parent.selectionStart === parent.selectionEnd
        color: parent.placeholderTextColor
        width: 1

        Timer {
            // This is duped from CursorDelegate.qml to preserve cursor blinking which is overwritten
            // when explicity setting cursor.color.
            id: timer
            running: cursor.parent.activeFocus && !cursor.parent.readOnly && interval != 0
            repeat: true
            interval: Qt.styleHints.cursorFlashTime / 2
            onTriggered: cursor.opacity = !cursor.opacity ? 1 : 0
            // force the cursor visible when gaining focus
            onRunningChanged: cursor.opacity = 1
        }
    }
