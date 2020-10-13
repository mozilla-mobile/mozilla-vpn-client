/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15

import "../themes/themes.js" as Theme

RoundButton {
    required property var labelText
    property variant fontName: Theme.fontInterFamily
    property var linkColor: Theme.blueButton
    property var isBoldLink: false
    signal clicked

    id: root
    radius: 4
    height: 28

    background: Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: Theme.bgColor
        radius: 4
        border.width: root.activeFocus ? 2 : 0
        border.color: Theme.blueFocusStroke
    }

    contentItem: Label {
        id: label
        text: labelText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: Theme.fontSize
        font.family: fontName

        Component.onCompleted: {
            if (isBoldLink) {
                font.weight = Font.Bold
            }
        }

        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
    }

    state: "state-default"

    states: [
        State {
            name: "state-hovering"
            PropertyChanges {
                target: label
                color: root.linkColor.buttonHovered
            }
        },
        State {
            name: "state-pressed"
            PropertyChanges {
                target: label
                color: root.linkColor.buttonPressed
            }
        },
        State {
            name: "state-default"
            PropertyChanges {
                target: label
                color: root.linkColor.defaultColor
            }
        }
    ]

    focusPolicy: Qt.StrongFocus

    Keys.onSpacePressed: clicked()
    Keys.onReturnPressed: clicked()
    Accessible.name: labelText
    Accessible.onPressAction: clicked()

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onEntered: root.state = "state-hovering"
        onExited: root.state = "state-default"
        onPressed: root.state = "state-pressed"
        onClicked: parent.clicked()
        cursorShape: "PointingHandCursor"
    }
}
