/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

RadioDelegate {
    id: radioControl

    property bool isHoverable: true
    property var radioButtonLabelText

    signal clicked()

    ButtonGroup.group: radioButtonGroup
    width: parent.width
    height: 40
    state: "state-default"
    states: [
        State {
            name: "state-pressed"

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? Theme.bluePressed : "#C2C2C2"
                scale: 0.55
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? Theme.bluePressed : "#3D3D3D"
            }

        },
        State {
            name: "state-default"

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? Theme.blue : Theme.bgColor
                scale: 0.6
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? Theme.blue : Theme.fontColor
            }

        },
        State {
            name: "state-hovering"

            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? Theme.bluePressed : Theme.greyHovered
            }

            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? Theme.bluePressed : Theme.fontColor
            }

        }
    ]

    VPNRadioButtonLabel {
        id: radioButtonLabel

        text: radioButtonLabelText
    }

    MouseArea {
        id: radioMouseArea

        anchors.fill: radioControl
        hoverEnabled: isHoverable
        onEntered: radioControl.state = "state-hovering"
        onExited: radioControl.state = "state-default"
        onPressed: radioControl.state = "state-pressed"
        onClicked: parent.clicked()
    }

    background: Rectangle {
        color: "transparent"
    }

    indicator: Rectangle {
        id: radioButton

        anchors.left: parent.left
        implicitWidth: 20
        implicitHeight: 20
        radius: implicitWidth * 0.5
        border.width: 2
        color: Theme.bgColor

        Rectangle {
            id: radioButtonInsetCircle

            anchors.fill: parent
            radius: radioButton.implicitHeight / 2
            scale: 0.6

            Behavior on color {
                ColorAnimation {
                    duration: 300
                }

            }

        }

        Behavior on border.color {
            ColorAnimation {
                duration: 300
            }

        }

    }

}
