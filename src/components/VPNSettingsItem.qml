import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

RowLayout {
    property alias text: item.text
    signal clicked

    id: root
    width: parent.width
    Layout.fillWidth: true

    VPNChevron {
        Layout.alignment: Qt.AlignTop
    }

    Label {
        id: item
        font.pixelSize: 15
        font.family: vpnFont.name
    }

    Item {
        Layout.fillWidth: true
    }

    VPNChevron {
        MouseArea {
            id: iconMouseArea
            anchors.fill: parent
            onClicked: root.clicked()
        }
    }
}
