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
    }
    height: 40
    width: 40
    states: [
        State {
            when: mouseArea.pressed
            PropertyChanges {
                target: backgroundRect
                color: backgroundColor.buttonPressed
            }
        },

        State {
            when:mouseArea.containsMouse
            PropertyChanges {
               target: backgroundRect
               color: backgroundColor.buttonHovered
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
        }

    ]
}
