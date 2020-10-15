/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.15
import "../themes/themes.js" as Theme

RoundButton {
    property variant targetEl
    property var enableHover: true
    property var bgColor: Theme.blueButton

    enabled: enableHover
    focusPolicy: Qt.StrongFocus
    Keys.onSpacePressed: clicked()
    Keys.onReturnPressed: clicked()
    Accessible.onPressAction: clicked()
    state: "state-default"
    states: [
        State {
            name: "state-default"

            PropertyChanges {
                target: targetEl
                color: bgColor.defaultColor
            }

        },
        State {
            name: "state-hovering"

            PropertyChanges {
                target: targetEl
                color: bgColor.buttonHovered
            }

        },
        State {
            name: "state-pressed"

            PropertyChanges {
                target: targetEl
                color: bgColor.buttonPressed
            }

        }
    ]

    MouseArea {
        id: mouseArea

        function changeState(stateName) {
            if (enableHover)
                parent.state = stateName;

        }

        anchors.fill: parent
        hoverEnabled: enableHover
        onEntered: changeState("state-hovering")
        onExited: changeState("state-default")
        onPressed: changeState("state-pressed")
        onClicked: {
            if (enableHover)
                parent.clicked();

        }
    }

    background: Rectangle {
        color: "transparent"
    }

}
