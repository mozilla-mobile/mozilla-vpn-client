/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import "../themes/themes.js" as Theme

Row {
    property var markerLabel
    property var rectColor
    property var markerData

    function computeRange() {
        if (markerData < 1.28e+05) {
            //% "Kbps"
            //: Kilobits per Secound
            return qsTrId("vpn.connectionInfo.kbps");
        }

        if (markerData < 1.28e+08) {
            //% "Mbps"
            //: Megabits per Second
            return qsTrId("vpn.connectioInfo.mbps");
        }

        //% "Gbps"
        //: Gigabits per Second
        return qsTrId("vpn.connectionInfo.gbps");
    }

    function roundValue(value) {
        return Math.round(value * 100) / 100;
    }

    function computeValue() {
        if (markerData < 128000)
            return roundValue(markerData / 8192);

        if (markerData < 1.28e+08)
            return roundValue(markerData / 8.192e+06);

        return roundValue(markeData / 8.192e+09);
    }
    Accessible.focusable: true
    Accessible.role: Accessible.StaticText
    Accessible.name: markerLabel
    Accessible.description: markerData
    focus: true
    activeFocusOnTab: true

    spacing: 12

    Rectangle {
        height: 12
        width: 12
        radius: 2
        color: rectColor
        anchors.top: parent.top
        anchors.topMargin: 22
    }

    Column {
        spacing: 6

        Text {
            font.pixelSize: 10
            height: 16
            text: computeRange()
            font.family: Theme.fontInterFamily
            color: "#FFFFFF"
        }

        Text {
            font.pixelSize: 14
            text: markerLabel
            font.family: Theme.fontBoldFamily
            color: "#FFFFFF"
        }

        Text {
            font.pixelSize: 16
            text: computeValue()
            font.family: Theme.fontInterFamily
            color: "#FFFFFF"
        }

    }

}
