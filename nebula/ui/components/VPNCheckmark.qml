/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

Item {
    id: checkmark

    height: VPNTheme.theme.iconSize * 1.5
    width: VPNTheme.theme.iconSize * 1.5

    Rectangle {
        id: checkmarkBg

        anchors.fill: parent
        antialiasing: true
        smooth: true
        visible: false
        color: VPNTheme.theme.transparent

        Behavior on color {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    Image {
        id: checkmarkIcon
        source: "qrc:/nebula/resources/checkmark-blue50.svg"
        sourceSize.height: checkmarkIcon.height
        sourceSize.width: checkmarkIcon.width
        opacity: checkBox.checked ? 1 : 0
        anchors.centerIn: parent
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }
    Behavior on opacity {
        PropertyAnimation {
            duration: 100
        }
    }
}
