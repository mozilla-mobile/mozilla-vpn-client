import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11

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
        font.weight: Font.Bold
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
