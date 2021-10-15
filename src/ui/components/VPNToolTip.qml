/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import compat 0.1
import themes 0.1

ToolTip {
    id: toolTip

    visible: mouseArea.containsMouse || parent.activeFocus
    onVisibleChanged: if (visible) fadeDown.start()
    leftMargin: Theme.windowMargin * 1.5
    rightMargin: Theme.windowMargin * 1.5
    delay: 1500
    leftPadding: 6
    rightPadding: 6
    topPadding: 4
    bottomPadding: 4
    closePolicy: ToolTip.CloseOnPressOutsideParent
    timeout: 5000

    ParallelAnimation {
        id: fadeDown

        PropertyAnimation {
            target: toolTip
            property: "opacity"
            from: 0
            to: 1
            duration: 200
        }

        PropertyAnimation {
            target: toolTip
            property: "y"
            from: parent.height + 3
            to: parent.height + 6
            duration: 200
        }

    }

    contentItem: Text {
        id: toolTipText

        text: toolTip.text
        color: Theme.fontColorDark
        z: 1
        wrapMode: Text.WordWrap
    }

    background: Rectangle {
        id: glowClippingPath

        radius: 4
        color: Theme.bgColor
        z: -1

        VPNRectangularGlow {
            anchors.fill: glowClippingPath
            glowRadius: 2
            spread: 0.5
            color: "#0C0C0D"
            cornerRadius: glowClippingPath.radius + glowRadius
            opacity: 0.1
            z: -2
        }

    }

}
