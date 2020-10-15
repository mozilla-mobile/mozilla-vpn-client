/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../themes/themes.js" as Theme

Button {
    property alias buttonText: buttonText.text
    property alias buttonTextColor: buttonText.color
    property var buttonColor

    id: popupButton
    enabled: (removePopup.state === "visible")
    Layout.preferredHeight: 28
    Layout.fillWidth: true
    Layout.preferredWidth: rectangularGlowClippingPath.width - (16 * 2)
    contentItem: Text {
        id: buttonText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: Theme.fontBoldFamily
        opacity: popupButton.hovered ? .9 : 1
        transitions: Transition {
            NumberAnimation {
                target: buttonText
                property: "opacity"
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }
    background: Rectangle {
        id: buttonBackgroundColor
        radius: 5
        color: buttonColor.defaultColor

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: popupButton.clicked()
            propagateComposedEvents: (removePopup.state === "visible")
        }

        states: [
            State {
                when: mouseArea.containsMouse
                PropertyChanges {
                    target: buttonBackgroundColor
                    color: buttonColor.buttonHovered
                }
            },
            State {
                when: mouseArea.pressed
                PropertyChanges {
                    target: buttonBackgroundColor
                    color: buttonColor.buttonPressed
                }
            }
        ]
        transitions: Transition {
            ColorAnimation {
                target: buttonBackgroundColor
                property: "color"
                duration: 200
            }
        }
    }
}
