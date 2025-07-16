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
    property var connectionTime: formatTime(VPNController.connectionTimestamp)

    function formatSingle(value) {
        if (value === 0)
            return "00";

        return (value < 10 ? "0" : "") + value;
    }

    function formatTime(timestamp) {
        var duration = 0;
        if (timestamp > 0) {
            duration = Math.floor((Date.now() - timestamp) / 1000);
        }

        var secs = duration % 60;
        duration = Math.floor(duration / 60);
        var mins = duration % 60;
        duration = Math.floor(duration / 60);
        return formatSingle(duration) + ":" + formatSingle(mins) + ":" + formatSingle(secs);
    }

    Timer {
        interval: 1000;
        running: VPNController.connectionTimestamp != 0;
        repeat: true

        onTriggered: connectionTime = formatTime(VPNController.connectionTimestamp)
    }

    Repeater{
        model: connectionTime.split("")

        Text {
            id: digit

            color: MZTheme.colors.fontColorInverted
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
