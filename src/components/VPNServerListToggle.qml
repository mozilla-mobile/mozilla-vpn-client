import QtQuick 2.0
import QtQuick.Layouts 1.11

// VPNServerListToggle
VPNIcon {
    source: "../resources/arrow-toggle.svg"
    transformOrigin: Image.Center
    smooth: true
    rotation: -90

    states: State {
        name: "rotated"
        when: serverCountry.cityListVisible
        PropertyChanges {
            target: serverListToggle
            rotation: 0
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
