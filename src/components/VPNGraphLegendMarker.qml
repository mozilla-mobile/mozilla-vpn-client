import QtQuick 2.0

Row {
    property var markerLabel
    property var rectColor
    property var markerData

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
            text: "Mbps"
            font.family: vpnFontInter.name
            color: "#FFFFFF"
        }
        Text {
            font.pixelSize: 14
            text: markerLabel
            font.family: vpnFont.name
            font.weight: Font.Bold
            color: "#FFFFFF"
        }
        Text {
            font.pixelSize: 16
            text: markerData
            font.family: vpnFontInter.name
            color: "#FFFFFF"
        }
    }
}
