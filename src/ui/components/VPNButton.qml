/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../themes/themes.js" as Theme

RoundButton {
    id: button

    height: 40
    width: Math.min(parent.width * .83, Theme.maxHorizontalContentWidth)
    anchors.horizontalCenter: parent.horizontalCenter
    Keys.onReturnPressed: clicked()
    Accessible.onPressAction: clicked()
    state: "state-default"
    states: [
        State {
            name: "state-default"

            PropertyChanges {
                target: bgColor
                color: Theme.blueButton.defaultColor
            }

        },
        State {
            name: "state-hovering"

            PropertyChanges {
                target: bgColor
                color: Theme.blueButton.buttonHovered
            }

        },
        State {
            name: "state-pressed"

            PropertyChanges {
                target: bgColor
                color: Theme.blueButton.buttonPressed
            }

        }
    ]

    VPNFocus {
        itemToFocus: button
        anchors.margins: -4
        radius: 6
        focusWidth: 5
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: button.state = "state-hovering"
        onExited: button.state = "state-default"
        onPressed: button.state = "state-pressed"
        onClicked: button.clicked()
    }

    background: Rectangle {
        id: bgColor

        color: Theme.blueButton.defaultColor
        radius: 4

        Behavior on color {
            ColorAnimation {
                duration: 200
            }

        }

    }

    contentItem: Label {
        id: label

        color: "#FFFFFF"
        text: button.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.family: Theme.fontBoldFamily
        font.pixelSize: 15
    }

}
