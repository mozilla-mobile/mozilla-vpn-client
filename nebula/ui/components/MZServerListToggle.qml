/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

// MZServerListToggle
MZIcon {
    source: "qrc:/nebula/resources/arrow-toggle.svg"
    transformOrigin: Image.Center
    smooth: true
    rotation: -90
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

    states: State {
        name: "rotated"
        when: serverCountry.cityListVisible

        PropertyChanges {
            target: serverListToggle
            rotation: 0
        }

    }

}
