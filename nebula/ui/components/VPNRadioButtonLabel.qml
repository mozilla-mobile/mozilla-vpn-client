/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

// VPNRadioButtonLabel
Label {
    anchors.left: radioButton.right
    anchors.leftMargin: VPNTheme.theme.hSpacing - 2
    font.family: VPNTheme.theme.fontInterFamily
    font.pixelSize: VPNTheme.theme.fontSize
    color: VPNTheme.theme.fontColorDark

    states: State {
        when: radioControl.checked

        PropertyChanges {
            target: radioButtonLabel
            color: VPNTheme.theme.blue
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
