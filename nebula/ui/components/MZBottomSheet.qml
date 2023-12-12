import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import components 0.1
import Mozilla.Shared 1.0

Drawer {
    id: drawer

    readonly property int maxHeight: parent.height - MZTheme.theme.sheetTopMargin //parent is the overlay

    implicitWidth: window.width
    implicitHeight: maxHeight

    topPadding: 0

    dragMargin: 0
    edge: Qt.BottomEdge
    background: Rectangle {

        radius: 8
        color: MZTheme.theme.bgColor

        Rectangle {
            color: parent.color
            anchors.bottom: parent.bottom
            width: parent.width
            height: parent.radius
        }
    }

    Overlay.modal: Rectangle {
        color: MZTheme.theme.overlayBackground
    }
}
