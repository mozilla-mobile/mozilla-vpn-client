/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtCharts 2.0
import QtQuick.Controls 2.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme


Item {
    property var rBytes: VPNConnectionData.rxBytes
    property var tBytes: VPNConnectionData.txBytes
    property var latencyMSec: VPNConnectionHealth.latency

    id: connectionInfo
    anchors.fill: box

    function open() {
        VPNConnectionData.activate(txSeries, rxSeries, axisX, axisY);
        VPNCloseEventHandler.addView(connectionInfo);
        connectionInfo.forceActiveFocus()
        connectionInfo.visible = true;
    }

    function close() {
        VPNConnectionData.deactivate();
        connectionInfo.visible = false;
    }

    Rectangle {
        id: rect

        anchors.fill: parent
        color: boxBackground.color
        radius: boxBackground.radius

    }
    ChartView {
        id: chart

        antialiasing: true
        backgroundColor: "#321C64"
        width: connectionInfo.width
        height: (connectionInfo.height / 2) - 32
        legend.visible: false
        anchors.top: connectionInfo.top
        anchors.topMargin: 64
        anchors.left: connectionInfo.left
        anchors.right: connectionInfo.right
        margins.top: 0
        margins.bottom: 0
        margins.left: 0
        margins.right: 0
        animationOptions: ChartView.NoAnimation

        Accessible.focusable: true
        Accessible.role: Accessible.Dialog
        Accessible.name: connectionInfoButton.accessibleName


        ValueAxis {
            id: axisX

            tickCount: 1
            min: 0
            max: 29
            lineVisible: false
            labelsVisible: false
            gridVisible: false
            visible: false
        }

        ValueAxis {
            id: axisY

            tickCount: 1
            min: 10
            max: 80
            lineVisible: false
            labelsVisible: false
            gridVisible: false
            visible: false
        }

        SplineSeries {
            id: txSeries

            axisX: axisX
            axisY: axisY
            color: "#F68953"
            width: 2
        }

        SplineSeries {
            id: rxSeries

            axisX: axisX
            axisY: axisY
            color: "#EE3389"
            width: 2
        }

    }

    Column {
        spacing: 12
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin * 2
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - ((backButton.anchors.leftMargin + backButton.width) * 2)

        VPNIPAddress {
            //% "IPv4:"
            //: The abbreviation for Internet Protocol. This is followed by the user’s IPv4 address.
            property var ipv4label: qsTrId("vpn.connectionInfo.ipv4")

            //% "IP:"
            //: The abbreviation for Internet Protocol. This is followed by the user’s IP address.
            property var iplabel: qsTrId("vpn.connectionInfo.ip2")

            visible: VPNConnectionData.ipv4Address !== ""
            ipVersionText: VPNConnectionData.ipv6Address === "" ? iplabel : ipv4label;
            ipAddressText: VPNConnectionData.ipv4Address
        }

        VPNIPAddress {
            visible: VPNConnectionData.ipv6Address !== ""
            //% "IPv6:"
            //: The abbreviation for Internet Procol version 6. This is followed by the user’s IPv6 address.
            ipVersionText: qsTrId("vpn.connectionInfo.ipv6")
            ipAddressText: VPNConnectionData.ipv6Address
        }
    }
    Row {
        spacing: 48
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        anchors.horizontalCenter: parent.horizontalCenter

        Column {
            spacing: 6

            Accessible.focusable: true
            Accessible.role: Accessible.StaticText
            //% "%1: %2 %3"
            //: Used as accessibility description for the connection info:
            //: %1 is the localized label for “Ping”, %2 is the rtt delay,
            //: %3 is the localized unit. Example: “Ping: 123 ms”.
            Accessible.name: qsTrId("vpn.connectionInfo.accessibleName")
                .arg(pingRange.text)
                .arg(pingLabel.text)
                .arg(pingValue.text)

            Text {
                id: pingRange

                font.pixelSize: 10
                height: 16
                //% "ms"
                //: milliseconds
                text: qsTrId("vpn.connectionInfo.ms");
                font.family: Theme.fontInterFamily
                color: "#FFFFFF"
            }

            Text {
                id: pingLabel

                font.pixelSize: 14
                //% "Ping"
                //: The current latency to the VPN server. The latency is shown on the next line.
                text: qsTrId("vpn.connectionInfo.ping")
                font.family: Theme.fontBoldFamily
                color: "#FFFFFF"
            }

            Text {
                id: pingValue

                font.pixelSize: 16
                text: connectionInfo.latencyMSec
                font.family: Theme.fontInterFamily
                color: "#FFFFFF"
            }
        }

        VPNGraphLegendMarker {
            //% "Download"
            //: The current download speed. The speed is shown on the next line.
            markerLabel: qsTrId("vpn.connectionInfo.download")
            rectColor: "#EE3389"
            markerData: connectionInfo.rBytes
        }

        VPNGraphLegendMarker {
            //% "Upload"
            //: The current upload speed. The speed is shown on the next line.
            markerLabel: qsTrId("vpn.connectionInfo.upload")
            rectColor: "#F68953"
            markerData: connectionInfo.tBytes
        }

    }

    VPNIconButton {
        id: backButton
        objectName: "connectionInfoBackButton"

        onClicked: {
            connectionInfo.close();
        }

        buttonColorScheme: Theme.iconButtonDarkBackground
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        //% "Close"
        accessibleName: qsTrId("vpn.connectionInfo.close")
        enabled: connectionInfo.visible

        Image {
            anchors.centerIn: backButton
            source: "../resources/close-white.svg"
            sourceSize.height: 16
            sourceSize.width: 16
        }

    }

    Connections {
        function onGoBack(item) {
            if (item === connectionInfo)
                backButton.clicked();

        }

        target: VPNCloseEventHandler
    }
}
