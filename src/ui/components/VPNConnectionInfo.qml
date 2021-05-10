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
        color:"#321C64"
        radius: box.radius

    }
    ChartView {
        id: chart

        antialiasing: true
        backgroundColor: "#321C64"
        width: connectionInfo.width
        height: (connectionInfo.height / 2) - 32
        legend.visible: false
        anchors.horizontalCenter: connectionInfo.horizontalCenter
        anchors.top: connectionInfo.top
        anchors.topMargin: 48
        anchors.left: connectionInfo.left
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

    VPNBoldLabel {
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin * 1.5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.horizontalCenter: parent.center
        anchors.horizontalCenterOffset: 0
        horizontalAlignment: Text.AlignHCenter
        color: Theme.white
        //% "IP: %1"
        //: The current IP address
        text: qsTrId("vpn.connectionInfo.ip").arg(VPNConnectionData.ipAddress)
        Accessible.name: text
        Accessible.role: Accessible.StaticText
    }

    Row {
        spacing: 48
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        anchors.horizontalCenter: parent.horizontalCenter

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
