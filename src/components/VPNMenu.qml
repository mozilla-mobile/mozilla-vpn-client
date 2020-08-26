import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

Item {
    property alias title: title.text
    property alias rightTitle: rightTitle.text

    id: menuBar
    width: parent.width
    height: 56

    Image {
        id: backImage
        source: "../resources/back.svg"
        sourceSize.width: Theme.iconSize

        fillMode: Image.PreserveAspectFit
        anchors.top: menuBar.top
        anchors.verticalCenter: menuBar.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: Theme.hSpacing

        MouseArea {
            anchors.fill: backImage
            onClicked: stackview.pop()
        }
    }

    VPNBoldLabel {
        id: title
        anchors.top: menuBar.top
        anchors.centerIn: menuBar
    }

    VPNLightLabel {
        id: rightTitle
        anchors.verticalCenter: menuBar.verticalCenter
        anchors.right: menuBar.right
        anchors.rightMargin: Theme.hSpacing
    }

    Rectangle {
        color: "#0C0C0D0A"
        y: 55
        width: parent.width
        height: 1
    }
}
