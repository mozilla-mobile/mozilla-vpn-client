/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// IMPORTANT: this file is used only for mobile builds.

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNButtonBase {
    id: base

    property var buttonText
    property var iconSource

    Layout.alignment: Qt.AlignCenter
    Layout.preferredHeight: parent.height
    Layout.fillWidth: true
    radius: 0

    Accessible.name: buttonText

    VPNUIStates {
        colorScheme: VPNTheme.theme.iconButtonLightBackground
        radius: 0
    }

    VPNMouseArea {
        id: mouseArea
    }

    contentItem: Item {
        anchors.fill: parent

        ColumnLayout {
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                source: iconSource
                sourceSize.height: 24
                sourceSize.width: 24
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: buttonText
                color: VPNTheme.theme.fontColor
                font.pixelSize: 11
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                font.family: VPNTheme.theme.fontInterFamily
                lineHeight: 18
                lineHeightMode: Text.FixedHeight
            }

        }

    }

}

