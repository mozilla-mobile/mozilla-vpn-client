/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import "../themes/themes.js" as Theme

RoundButton {
    id: root

    required property var labelText
    property variant fontName: Theme.fontInterFamily
    property var linkColor: Theme.blueButton

    signal clicked()

    radius: 4
    height: 28
    state: "state-default"
    focusPolicy: Qt.StrongFocus
    onFocusChanged: if (focus && typeof(ensureVisible) !== "undefined") ensureVisible(root)


    Keys.onSpacePressed: clicked()
    Keys.onReturnPressed: clicked()
    Accessible.name: labelText
    Accessible.onPressAction: clicked()
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

    VPNFocus {
        itemToFocus: root
    }

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

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent
        color: Theme.bgColor
        radius: 4
    }

    contentItem: Label {
        id: label

        text: labelText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: Theme.fontSize
        font.family: fontName

        Behavior on color {
            ColorAnimation {
                duration: 200
            }

        }

    }

}
