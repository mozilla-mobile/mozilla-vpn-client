/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtCharts 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme


Popup {
    id: popup

    height: box.height
    width: box.width
    padding: 0
    leftInset: 0
    rightInset: 0
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    onClosed: VPNConnectionData.deactivate()
    onOpened: {
        VPNConnectionData.activate(txSeries, rxSeries, axisX, axisY);
        VPNCloseEventHandler.addView(chartWrapper);
        popup.forceActiveFocus()
    }

    // TODO: We can not use Accessible type on Popup because it does not inherit
    // from an Item. The code below generates the following warning:
    // "...Accessible must be attached to an Item..."
    // See https://github.com/mozilla-mobile/mozilla-vpn-client/issues/322 for
    // more details.
    // Accessible.focusable: true
    // Accessible.role: Accessible.Dialog
    // Accessible.name: connectionInfoButton.accessibleName


    contentItem: Item {
        id: chartWrapper

        property var rBytes: VPNConnectionData.rxBytes
        property var tBytes: VPNConnectionData.txBytes

        width: box.width
        height: box.height
        antialiasing: true

        ChartView {
            id: chart

            antialiasing: true
            backgroundColor: "#321C64"
            width: chartWrapper.width
            height: (chartWrapper.height / 2) - 32
            legend.visible: false
            anchors.horizontalCenter: chartWrapper.horizontalCenter
            anchors.top: chartWrapper.top
            anchors.topMargin: 48
            anchors.left: chartWrapper.left
            margins.top: 0
            margins.bottom: 0
            margins.left: 0
            margins.right: 0
            animationOptions: ChartView.NoAnimation


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
                markerData: chartWrapper.rBytes
            }

            VPNGraphLegendMarker {
                //% "Upload"
                //: The current upload speed. The speed is shown on the next line.
                markerLabel: qsTrId("vpn.connectionInfo.upload")
                rectColor: "#F68953"
                markerData: chartWrapper.tBytes
            }

        }

        VPNIconButton {
            id: backButton

            onClicked: popup.close()
            buttonColorScheme: Theme.iconButtonDarkBackground
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: Theme.windowMargin / 2
            anchors.leftMargin: Theme.windowMargin / 2
            //% "Close"
            accessibleName: qsTrId("vpn.connectionInfo.close")

            Image {
                anchors.centerIn: backButton
                source: "../resources/close-white.svg"
                sourceSize.height: 16
                sourceSize.width: 16
            }

        }

        Connections {
            function onGoBack(item) {
                if (item === chartWrapper)
                    backButton.clicked();

            }

            target: VPNCloseEventHandler
        }

    }

    background: Rectangle {
        color: box.color
        height: box.height
        width: box.width
        radius: box.radius
        antialiasing: true
    }

    Overlay.modal: Rectangle {
        color: Theme.bgColor30
        opacity: 0.5
    }

}
