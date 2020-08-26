import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

// VPNRadioButton
Rectangle {
    id: radioButton
    anchors.left: parent.left
    anchors.leftMargin: flag.Layout.leftMargin + serverListToggle.Layout.preferredWidth

    implicitWidth: Theme.iconSize
    implicitHeight: Theme.iconSize

    radius: implicitWidth * 0.5
    border.color: Theme.fontColor
    border.width: 2
    color: Theme.bgColor

    Rectangle {
        id: radioButtonInsetCircle
        anchors.fill: parent
        color: Theme.buttonColor
        opacity: 0
        radius: 9
        anchors.margins: 4

        states: [
            State {
                when: control.checked
                PropertyChanges {
                    target: radioButtonInsetCircle
                    opacity: 1
                }
            }
            // TODO Add hover states
        ]

        transitions: Transition {
            NumberAnimation {
                target: radioButtonInsetCircle
                properties: "opacity"
                duration: 100
            }
        }
    }

    states: State {
        when: control.checked
        PropertyChanges {
            target: radioButton
            border.color: Theme.buttonColor
        }
    }

    transitions: Transition {
        ColorAnimation {
            target: radioButton
            properties: "border.color"
            duration: 100
        }
    }
}
