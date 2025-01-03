/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import compat 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZIcon {
    source: MZAssetLookup.getImageSource("ArrowToggle")
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
