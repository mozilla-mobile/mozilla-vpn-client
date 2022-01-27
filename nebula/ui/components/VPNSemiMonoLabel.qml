/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    property real defaultWidth: VPNTheme.theme.fontSize * 0.25
    property real narrowWidth: VPNTheme.theme.fontSize * 0.05
    property var narrowCharacters: [" ", "\t", "\n", ":"]

    Repeater {
        model: formatTime(VPNController.time).split("")

        Text {
            id: digit

            color: VPNTheme.theme.white
            horizontalAlignment: Text.AlignHCenter
            font.letterSpacing: 0
            font.pixelSize: VPNTheme.theme.fontSize
            text: modelData

            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.preferredWidth: narrowCharacters.includes(modelData) ? narrowWidth : defaultWidth
            Layout.preferredHeight: VPNTheme.theme.controllerInterLineHeight
        }
    }
}
