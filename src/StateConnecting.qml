import QtQuick 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0

Item {
    Text {
        id: getHelp
        color: "#0a68e3"
        text: qsTr("Connecting")
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 16
        anchors.rightMargin: 16
        MouseArea {
            onClicked: console.log("B")
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            hoverEnabled: true
        }
    }
}
