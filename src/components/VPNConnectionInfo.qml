import QtQuick 2.0
import QtCharts 2.0

import Mozilla.VPN 1.0

ChartView {
    theme: ChartView.ChartThemeBrownSand
    antialiasing: true
    legend.visible: false

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
