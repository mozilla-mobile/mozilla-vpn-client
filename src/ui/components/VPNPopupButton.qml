/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../themes/themes.js" as Theme

VPNButtonBase {
    id: button

    property alias buttonText: buttonText.text
    property alias buttonTextColor: buttonText.color

    targetEl: buttonBackground
    enabled: popup.visible
    Layout.fillWidth: true
    Layout.fillHeight: true

    Rectangle {
        id: buttonBackground

        anchors.fill: button
        height: button.contentItem.paintedHeight
        radius: 4
        color: bgColor.defaultColor
        border.width: 1
        border.color: button.activeFocus ? bgColor.focusBorder : color
        z: -1

        Rectangle {
            anchors.fill: buttonBackground
            anchors.margins: -2
            opacity: button.activeFocus ? 1 : 0
            radius: buttonBackground.radius + 2
            color: bgColor.focusStroke
            z: -2
        }

        Behavior on color {
            PropertyAnimation {
                target: buttonBackground
                duration: 300
            }

        }

    }

    contentItem: VPNInterLabel {
        id: buttonText

        lineHeight: 15
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        opacity: button.hovered ? 0.9 : 1

        transitions: Transition {
            NumberAnimation {
                target: buttonText
                property: "opacity"
                duration: 200
                easing.type: Easing.InOutQuad
            }

        }

    }

}
