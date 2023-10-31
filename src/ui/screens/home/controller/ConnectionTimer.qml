/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0

RowLayout {
    property real defaultWidth: MZTheme.theme.fontSize * 0.25
    property real narrowWidth: MZTheme.theme.fontSize * 0.05
    property var narrowCharacters: [" ", "\t", "\n", ":"]
    property bool ignoreForAccessibility: false

    Accessible.ignored: ignoreForAccessibility

    function formatSingle(value) {
        if (value === 0)
            return "00";

        return (value < 10 ? "0" : "") + value;
    }

    function formatTime(time) {
        var secs = time % 60;
        time = Math.floor(time / 60);
        var mins = time % 60;
        time = Math.floor(time / 60);
        return formatSingle(time) + ":" + formatSingle(mins) + ":" + formatSingle(secs);
    }

    Repeater {
        model: formatTime(VPNController.time).split("")

        Text {
            id: digit

            color: MZTheme.theme.white
            horizontalAlignment: Text.AlignHCenter
            font.letterSpacing: 0
            font.pixelSize: MZTheme.theme.fontSize
            text: modelData
            Accessible.ignored: ignoreForAccessibility || !visible

            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.preferredWidth: narrowCharacters.includes(modelData) ? narrowWidth : defaultWidth
            Layout.preferredHeight: MZTheme.theme.controllerInterLineHeight
        }
    }
}
