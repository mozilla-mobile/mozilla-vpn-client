/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

Item {
    property alias color: checkmarkBg.color

    id: checkmark

    height: 20
    width: 20

    Rectangle {
        id: checkmarkBg

        height: 20
        width: 20
        antialiasing: true
        smooth: true
        visible: false

        Behavior on color {
            PropertyAnimation {
                duration: 200
            }

        }

    }

    Image {
        id: checkmarkIcon

        source: "qrc:/nebula/resources/checkmark.svg"
        sourceSize.height: 13
        sourceSize.width: 12
        visible: false
        anchors.centerIn: checkmark
    }

    VPNOpacityMask {
        anchors.centerIn: checkmark
        height: checkmarkIcon.height
        width: checkmarkIcon.width
        source: checkmarkBg
        maskSource: checkmarkIcon
    }

    Behavior on opacity {
        PropertyAnimation {
            duration: 100
        }

    }

}