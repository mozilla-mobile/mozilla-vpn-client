import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

// VPNFooterLink
Text {
    signal clicked
    id: root
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 40
    font.pixelSize: Theme.fontSize
    font.family: vpnFontInter.name
    focus: true

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
