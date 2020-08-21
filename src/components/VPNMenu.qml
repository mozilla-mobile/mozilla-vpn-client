import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

Item {
    property alias title: title.text
    property alias rightTitle: rightTitle.text

    id: menuBar
    width: parent.width
    height: 40

    RowLayout {
        id: row
        width: parent.width
        Layout.fillWidth: true

        Image {
            id: backImage
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

        Item {
            Layout.fillWidth: true
        }

        Label {
            id: rightTitle
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 10
            Layout.topMargin: 4
        }
    }

    Label {
        id: title
        anchors.top: row.top
        anchors.centerIn: row
    }

    Rectangle {
        color: "#000000"
        anchors.top: row.bottom
        width: parent.width
        height: 1
    }
}
