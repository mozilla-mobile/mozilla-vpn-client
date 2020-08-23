import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

// ServerListToggle
Image {
    Layout.preferredHeight: 8
    Layout.preferredWidth: 8

    fillMode: Image.PreserveAspectFit
    smooth: true
    source: "../resources/toggle-down.svg"
    transformOrigin: Image.Center

    transitions: [
        Transition {
            from: ""
            to: "rotated"
            RotationAnimation {
                properties: "rotation"
                duration: 200
                direction: RotationAnimation.Clockwise
                easing.type: Easing.InOutQuad
            }
        },

        Transition {
            from: "rotated"
            to: ""
            RotationAnimation {
                properties: "rotation"
                duration: 200
                direction: RotationAnimation.Counterclockwise
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
