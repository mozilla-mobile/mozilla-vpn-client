import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0

import "../themes/themes.js" as Theme

//VPNHeaderLink
Text {
    signal clicked
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.topMargin: 16
    anchors.rightMargin: 16
    font.family: vpnFontInter.name
    font.pixelSize: 15

    color: {
        if (mouseArea.pressed) {
            return Theme.bluePressed
        }
        if (mouseArea.containsMouse) {
            return Theme.blueHovered
        }
        return Theme.blue
    }

    Behavior on color {
        ColorAnimation {
            duration: 200
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: parent.clicked()
        cursorShape: "PointingHandCursor"
        hoverEnabled: true
    }
}
