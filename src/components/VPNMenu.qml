import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

Item {
    property alias title: title.text
    property alias rightTitle: rightTitle.text

    id: menuBar
    width: parent.width
    height: 56

    Image {
        id: backImage
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 20
        anchors.leftMargin: 20
        source: "../resources/back.svg"
        sourceSize.width: 16
        sourceSize.height: 16

        MouseArea {
            anchors.fill: parent
            onClicked: stackview.pop()
        }
    }

    Label {
        id: title
        anchors.top: menuBar.top
        anchors.centerIn: menuBar
        font.pixelSize: 15
        font.family: vpnFont.name
    }

    Label {
        id: rightTitle
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 17
        anchors.rightMargin: 17
        font.pixelSize: 15
    }

    Rectangle {
        color: "#0C0C0D0A"
        y: 55
        width: parent.width
        height: 1
    }
}
