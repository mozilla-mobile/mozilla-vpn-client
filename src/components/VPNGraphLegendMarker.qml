/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import "../themes/themes.js" as Theme

Row {
    property var markerLabel
    property var rectColor
    property var markerData

    // TODO: check if these 3 functions do the correct math to compute the values.

    function computeRange() {
        if (markerData < 128000) {
            //% "Kbps"
            //: Kilobits per Secound
            return qsTrId("kbps");
        }

        if (markerData < 128000000) {
             //% "Mbps"
             //: Megabits per Second
            return qsTrId("mbps")
        }
        //% "Gbps"
        //: Gigabits per Second
        return qsTrId("gbps");
    }

    function roundValue(value) {
        return Math.round(value * 100) / 100
    }

    function computeValue() {
        if (markerData < 128000) {
            return roundValue(markerData / 8192);
        }

        if (markerData < 128000000) {
            return roundValue(markerData / 8192000);
        }

        return roundValue(markeData / 8192000000);
    }

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
