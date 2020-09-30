/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

RoundButton {
    id: iconButton
    property var backgroundColor: Theme.greyButton
    property var defaultColor: Theme.bgColor
    signal clicked

    background: Rectangle {
        id: backgroundRect
        radius: 4
        color: defaultColor
        opacity: 0
    }
    height: 40
    width: 40
    states: [
        State {
            when: mouseArea.pressed
            PropertyChanges {
                target: backgroundRect
                color: backgroundColor.buttonPressed
                opacity: 1
            }
        },

        State {
            when:mouseArea.containsMouse
            PropertyChanges {
               target: backgroundRect
               color: backgroundColor.buttonHovered
               opacity: 1
            }
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
    }


    transitions: [
        Transition {
            ColorAnimation {
                target: backgroundRect
                duration: 300
            }
            PropertyAnimation {
                target: backgroundRect
                property: "opacity"
                duration: 300
            }
        }

    ]
}
