import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

RadioDelegate {
    property bool radioControlIsBeingHovered: false

    id: radioControl
    ButtonGroup.group: radioButtonGroup
    background: Rectangle {
        color: "transparent"
    }

    checked: code === VPNCurrentServer.countryCode
             && cityName.text === VPNCurrentServer.city
    width: parent.width
    height: 40

    indicator: Rectangle {
        id: radioButton
        anchors.left: parent.left
        anchors.leftMargin: Theme.hSpacing + Theme.vSpacing + 14
        implicitWidth: 20
        implicitHeight: 20

        radius: implicitWidth * 0.5
        border.color: Theme.fontColor
        border.width: 2
        color: Theme.bgColor

        Rectangle {
            id: radioButtonInsetCircle
            anchors.fill: parent
            color: Theme.bgColor
            radius: radioButton.implicitHeight / 2
            scale: .6
        }
    }

    VPNRadioButtonLabel {
        id: cityName
    }

    states: [
        State {
            name: "radio-pressed"
            when: radioMouseArea.pressed
            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? Theme.bluePressed : "#C2C2C2"
                scale: .55
            }
            PropertyChanges {
                target: radioButton
                border.color: radioControl.checked ? Theme.bluePressed : "#3D3D3D"
            }
        },
        State {
            name: "radio-checked"
            when: radioControl.checked
            PropertyChanges {
                target: radioButtonInsetCircle
                color: Theme.blue
                scale: .6
            }
            PropertyChanges {
                target: radioButton
                border.color: Theme.blue
            }
        },
        State {
            name: "radio-hovered"
            when: radioControlIsBeingHovered
            PropertyChanges {
                target: radioButtonInsetCircle
                color: radioControl.checked ? Theme.bluePressed : Theme.greyHovered
            }
        }
    ]

    transitions: [
        Transition {
            to: "radio-pressed"
            ParallelAnimation {
                PropertyAnimation {
                    target: radioButtonInsetCircle
                    property: "scale"
                    duration: 100
                }
                ColorAnimation {
                    targets: [radioButtonInsetCircle, radioButton]
                    duration: 100
                }
            }
        },

        Transition {
            from: "radio-pressed"
            to: "radio-checked"

            ParallelAnimation {
                PropertyAnimation {
                    target: radioButtonInsetCircle
                    property: "scale"
                    to: .6
                    duration: 150
                }
                ColorAnimation {
                    targets: [radioButtonInsetCircle, radioButton]
                    duration: 150
                    from: Theme.bluePressed
                    to: Theme.blue
                }
            }
        },
        Transition {
            to: "radio-hovered"
            ColorAnimation {
                target: radioButtonInsetCircle
                duration: 200
            }
        }
    ]

    MouseArea {
        id: radioMouseArea
        anchors.fill: radioControl
        hoverEnabled: serverCountry.cityListVisible
        onEntered: {
            radioControlIsBeingHovered = true
        }
        onExited: {
            radioControlIsBeingHovered = false
        }
        onClicked: {
            VPNController.changeServer(code, cityName.text)
            stackview.pop()
        }
    }
}
