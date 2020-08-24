import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

RowLayout {
    property alias text: item.text
    property alias imageRight: imageRight.source
    property alias imageLeft: imageLeft.source
    signal clicked

    id: root
    width: parent.width
    Layout.fillWidth: true

    Layout.leftMargin: 16
    Layout.rightMargin: 16

    Image {
        id: imageLeft
        height: 18
        width: 18
        Layout.alignment: Qt.AlignTop
        sourceSize.width: 18
        sourceSize.height: 18
    }

    Label {
        id: item
        font.pixelSize: 15
        font.family: vpnFont.name
    }

    Item {
        Layout.fillWidth: true
    }

    Image {
        id: imageRight
        height: 18
        width: 18
        Layout.alignment: Qt.AlignTop
        sourceSize.width: 18
        sourceSize.height: 18

        MouseArea {
            id: iconMouseArea
            anchors.fill: parent
            onClicked: root.clicked()
        }
    }
}
