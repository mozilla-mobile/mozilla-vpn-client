import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.0

RoundButton {
    id: button

    height: 40
    width: parent.width

    Layout.preferredHeight: 40
    Layout.fillWidth: true

    Layout.leftMargin: 16
    Layout.rightMargin: 16

    background: Rectangle {
        color: "#0060DF"
        radius: 4
    }

    contentItem: Label {
        id: label
        color: "#FFFFFF"
        text: button.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.family: vpnFont.name
        font.pixelSize: 15
        font.weight: Font.Bold
    }
}
