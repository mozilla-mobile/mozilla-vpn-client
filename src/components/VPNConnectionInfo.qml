import QtQuick 2.0
import QtCharts 2.0

import Mozilla.VPN 1.0

ChartView {
    theme: ChartView.ChartThemeBrownSand
    antialiasing: true

    ValueAxis {
        id: axisX
        min: 0
        max: 100
        tickCount: 5
    }

    ValueAxis {
        id: axisY
        min: 0
        max: 100000
    }

    SplineSeries {
        id: connectionInfoSeries
        axisX: axisX
        axisY: axisY
    }

    Component.onCompleted: {
        VPNConnectionData.setComponents(connectionInfoSeries, axisX, axisY);
    }
}
