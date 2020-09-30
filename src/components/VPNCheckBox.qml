/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

CheckBox {
    signal clicked

    id: checkBox
    height: 20
    width: 20
    Layout.alignment: Qt.AlignTop
    indicator: Rectangle {
        id: checkBoxIndicator
        height: 20
        width: 20
        color: "transparent"
        border.color: Theme.fontColor
        border.width: 2
        radius: 4
        antialiasing: true

        Behavior on border.color {
            PropertyAnimation {
                duration: 200
            }
        }
    }

    Item {
        property var checkmarkColor: "#DBDBDB"

        id: checkmark
        height: 20
        width: 20
        anchors.fill: checkBoxIndicator

        Behavior on opacity {
            PropertyAnimation {
                duration: 200
            }
        }

        Rectangle {
            id: checkmarkBg
            color: checkmark.checkmarkColor
            height: 20
            width: 20
            antialiasing: true
            smooth: true
            visible: false

            Behavior on color {
                PropertyAnimation {
                    duration: 200
                }
            }
        }

        Image {
            id: checkmarkIcon
            source: "../resources/checkmark.svg"
            sourceSize.height: 13
            sourceSize.width: 12
            visible: false
            anchors.centerIn: checkmark
        }

        OpacityMask {
            anchors.centerIn: checkmark
            height: checkmarkIcon.height
            width: checkmarkIcon.width
            source: checkmarkBg
            maskSource: checkmarkIcon
        }
    }

    MouseArea {
        id: checkBoxMouseArea
        hoverEnabled: true
        height: 20
        width: 20
        propagateComposedEvents: true
        onEntered: checkBox.state = "state-hovering"
        onExited: checkBox.state = "state-default"
        onReleased: if (checkBox.checked) { return checkBox.state = "state-default"; }
        onPressed: checkBox.state = "state-pressed"

        onClicked: {
            checkBox.clicked()
            mouse.accepted = false;
        }
    }

    state: "state-default"

    states: [
        State {
          name: "state-pressed"
          PropertyChanges {
              target: checkBoxIndicator
              border.color: checkBox.checked ? Theme.bluePressed : Theme.fontColorDark
          }
          PropertyChanges {
              target: checkmark
              opacity: 1
              checkmarkColor: checkBox.checked ? Theme.bluePressed : Theme.greyPressed
          }
        },
        State {
          name: "state-hovering"
          PropertyChanges {
              target: checkBoxIndicator
              border.color: checkBox.checked ? Theme.blueHovered : Theme.fontColorDark
          }
          PropertyChanges {
              target: checkmark
              opacity: 1
              checkmarkColor: checkBox.checked ? Theme.blueHovered : "#DBDBDB"
          }
        },
        State {
            name: "state-default"
            PropertyChanges {
                target: checkBoxIndicator
                border.color: checkBox.checked ? Theme.blue : Theme.fontColor
            }
            PropertyChanges {
                target: checkmark
                opacity: checkBox.checked ? 1 : 0
                checkmarkColor: checkBox.checked ? Theme.blue : "#DBDBDB"
            }
        }
    ]
}
