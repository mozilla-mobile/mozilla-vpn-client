import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

Item {
    ColumnLayout {
        id: menuBar
        width: parent.width
        Layout.fillWidth: true

        RowLayout {
            width: parent.width
            Layout.fillWidth: true

            Image {
                id: backImage
                height: 12
                width: 12
                Layout.leftMargin: 10
                Layout.topMargin: 10
                Layout.bottomMargin: 5
                Layout.alignment: Qt.AlignTop
                source: "../resources/back.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: stackview.pop()
                }
            }

            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.topMargin: 4
                text: qsTr("Settings")
            }
        }

        Rectangle {
            color: "#000000"
            width: parent.width
            height: 1
        }
    }
}
