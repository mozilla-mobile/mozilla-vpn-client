import QtQuick 2.0
import QtCharts 2.0

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

Item {
    id: chartWrapper
    anchors.fill: box

    states: [
        State {
            name: "visible"
            when: chartWrapper.visible
            StateChangeScript {
                script: VPNController.dataViewActive = true
            }
        },

        State {
            name: "invisible"
            when: !chartWrapper.visible
            StateChangeScript {
                script: VPNController.dataViewActive = false
            }
        }
    ]

    Rectangle {
        anchors.fill: parent
        height: parent.height
        width: parent.width
        color: "#321C64"
        radius: 8
        antialiasing: true

        ChartView {
            id: chart
            antialiasing: true
            backgroundColor: "#321C64"
            width: parent.width
            height: (parent.height / 2) - 32
            legend.visible: false
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 60
            margins.top: 0
            margins.bottom: 0
            margins.left: 0
            margins.right: 0
            animationOptions: ChartView.SeriesAnimations

            ValueAxis {
                id: axisX
                min: 0
                max: 29
                tickCount: 5
                lineVisible: false
                labelsVisible: false
                gridVisible: false
                visible: false
            }

            ValueAxis {
                id: axisY
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
                useOpenGL: false
                capStyle: Qt.RoundCap
                color: "#EE3389"
                width: 2
            }

            SplineSeries {
                id: rxSeries
                axisX: axisX
                axisY: axisY
                useOpenGL: true
                capStyle: Qt.RoundCap
                color: "#F68953"
                width: 2
            }

            Component.onCompleted: {
                VPNConnectionData.setComponents(txSeries, rxSeries,
                                                axisX, axisY)
            }
        }

        Text {
            anchors.top: parent.top
            anchors.topMargin: 24
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.horizontalCenter: parent.center
            anchors.horizontalCenterOffset: 0
            horizontalAlignment: Text.AlignHCenter
            color: "#FFFFFF"
            font.pixelSize: 15
            font.family: vpnFont.name
            font.weight: Font.Bold
            text: qsTr("IP:") + " " + VPNConnectionData.ipAddress
        }

        Row {
            spacing: 48
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 48
            anchors.horizontalCenter: parent.horizontalCenter

            VPNGraphLegendMarker {
                markerLabel: qsTr("Download")
                rectColor: "#EE3389"
                markerData: VPNConnectionData.rxBytes
            }

            VPNGraphLegendMarker {
                markerLabel: qsTr("Upload")
                rectColor: "#F68953"
                markerData: VPNConnectionData.txBytes
            }
        }

        VPNIconButton {
            id: backButton
            onClicked: chartWrapper.visible = false
            defaultColor: box.color
            backgroundColor: Theme.whiteSettingsBtn
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 8
            anchors.leftMargin: 8

            Image {
                anchors.centerIn: backButton
                source: "../resources/close-white.svg"
                sourceSize.height: 16
                sourceSize.width: 16
            }
        }
    }
}
