import QtQuick 2.0
import QtCharts 2.0

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

Item {
    ChartView {
        theme: ChartView.ChartThemeBrownSand
        antialiasing: true
        legend.visible: false
        anchors.fill: parent

        ValueAxis {
            id: axisX
            min: 0
            max: 30
            tickCount: 5
            lineVisible: false
            labelsVisible: false
            gridVisible: false
        }

        ValueAxis {
            id: axisY
            min: 0
            max: 100000
            lineVisible: false
            labelsVisible: false
            gridVisible: false
        }

        SplineSeries {
            id: txSeries
            axisX: axisX
            axisY: axisY
        }

        SplineSeries {
            id: rxSeries
            axisX: axisX
            axisY: axisY
        }

        Component.onCompleted: {
            VPNConnectionData.setComponents(txSeries, rxSeries, axisX, axisY);
        }
    }

    Text {
        text: VPNConnectionData.ipAddress
    }

    VPNIconButton {
        id: backButton
        onClicked: parent.visible = false
        defaultColor: box.color

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: Theme.windowMargin / 2
        anchors.rightMargin: Theme.windowMargin / 2

        Image {
            anchors.centerIn: backButton
            source: "../resources/settings.svg"
            sourceSize.height: 22
            sourceSize.width: 22
        }
    }
}
