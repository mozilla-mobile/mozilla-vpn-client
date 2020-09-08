import QtQuick 2.0
import QtQuick.Layouts 1.0

// VPNServerListToggle
Image {
    Layout.preferredHeight: 16
    Layout.preferredWidth: 16

    fillMode: Image.PreserveAspectFit
    smooth: true
    source: "../resources/arrow-toggle.svg"
    transformOrigin: Image.Center

    states: State {
        name: "rotated"
        when: serverCountry.cityListVisible
        PropertyChanges {
            target: serverListToggle
            rotation: 90
        }
    }

    transitions: [
        Transition {
            to: "rotated"
            RotationAnimation {
                properties: "rotation"
                duration: 200
                direction: RotationAnimation.Clockwise
                easing.type: Easing.InOutQuad
            }
        },

        Transition {
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
