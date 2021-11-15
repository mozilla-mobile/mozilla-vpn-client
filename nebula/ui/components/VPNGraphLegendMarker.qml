/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import themes 0.1

Row {
    property var markerLabel
    property var rectColor
    property var markerData

    function computeRange() {
        if (markerData < 1024) {
            //% "B/s"
            //: Bytes per second
            return qsTrId("vpn.connectionInfo.Bps");
        }

        if (markerData < 1048576 /* 1024^2 */) {
            //% "kB/s"
            //: Kilobytes per second
            return qsTrId("vpn.connectionInfo.kBps");
        }

        if (markerData < 1073741824 /* 1024^3 */) {
            //% "MB/s"
            //: Megabytes per second
            return qsTrId("vpn.connectioInfo.mBps");
        }

        if (markerData < 1099511627776 /* 1024^4 */) {
            //% "GB/s"
            //: Gigabytes per second
            return qsTrId("vpn.connectioInfo.gBps");
        }

        //% "TB/s"
        //: Terabytes per second
        return qsTrId("vpn.connectionInfo.tBps");
    }

    function roundValue(value) {
        return Math.round(value * 100) / 100;
    }

    function computeValue() {
        if (markerData < 1024)
            return roundValue(markerData);

        if (markerData < 1048576 /* 1024^2 */)
            return roundValue(markerData / 1024);

        if (markerData < 1073741824 /* 1024^3 */)
            return roundValue(markerData / 1048576 /* 1024^2 */);

        if (markerData < 1099511627776 /* 1024^4 */)
            return roundValue(markerData / 1073741824 /* 1024^3 */);

        return roundValue(markerData / 1099511627776 /* 1024^4 */);
    }
    Accessible.focusable: true
    Accessible.role: Accessible.StaticText
    //% "%1: %2 %3"
    //: Used as accessibility description for the connection info:
    //: %1 is the localized label for “Upload” or “Download”, %2 is the speed
    //: value, %3 is the localized unit. Example: “Upload: 10 Mbps”.
    Accessible.name: qsTrId("vpn.connectionInfo.accessibleName")
        .arg(label.text)
        .arg(value.text)
        .arg(range.text)

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
            id: range

            font.pixelSize: 10
            height: 16
            text: computeRange()
            font.family: Theme.fontInterFamily
            color: "#FFFFFF"
        }

        Text {
            id: label

            font.pixelSize: 14
            text: markerLabel
            font.family: Theme.fontBoldFamily
            color: "#FFFFFF"
        }

        Text {
            id: value

            font.pixelSize: 16
            text: computeValue()
            font.family: Theme.fontInterFamily
            color: "#FFFFFF"
        }

    }

}
