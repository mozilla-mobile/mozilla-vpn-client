/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import themes 0.1

// VPNRadioButtonLabel
Label {
    anchors.left: radioButton.right
    anchors.leftMargin: Theme.hSpacing - 2
    font.family: Theme.fontInterFamily
    font.pixelSize: Theme.fontSize
    color: Theme.fontColorDark

    states: State {
        when: radioControl.checked

        PropertyChanges {
            target: radioButtonLabel
            color: Theme.blue
        }

    }

    transitions: Transition {
        ColorAnimation {
            target: radioButtonLabel
            properties: "color"
            duration: 100
        }

    }

}
