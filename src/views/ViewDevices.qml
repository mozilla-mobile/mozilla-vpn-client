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
                text: qsTr("My devices")
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                text: qsTr("%1 of %2").arg(VPN.activeDevices).arg(VPN.maxDevices)
                Layout.rightMargin: 10
                Layout.topMargin: 4
            }
        }

        Rectangle {
            color: "#000000"
            width: parent.width
            height: 1
        }
    }

    ListView {
        height: parent.height - menuBar.height
        width: parent.width
        anchors.top: menuBar.bottom

        model: VPN.deviceModel

        delegate: ItemDelegate {
            id: device
            width: parent.width
            checkable: false

            contentItem: ColumnLayout {
                spacing: 0

                RowLayout {

                    Image {
                        id: deviceImage
                        height: 16
                        width: 16
                        Layout.alignment: Qt.AlignTop
                        source: "../resources/devices.svg"
                    }

                    ColumnLayout {
                        Label {
                            id: deviceName
                            text: name
                        }

                        Label {
                            id: deviceDesc
                            text: qsTr("Current device TODO")
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Image {
                        visible: !currentOne
                        id: deleteImage
                        height: 16
                        width: 16
                        Layout.alignment: Qt.AlignTop
                        source: "../resources/delete.svg"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: removePopup.initializeAndOpen(name)
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: removePopup
        modal: true
        margins: 16

        property var deviceName;

        function initializeAndOpen(name) {
            removePopup.deviceName = name;
            open();
        }

        contentItem: ColumnLayout {
            Label {
                text: qsTr("Remove device?")
            }
            Label {
                text: qsTr("Please confirm you would like to remove %1.").arg(removePopup.deviceName)
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 10
                Button {
                    text: qsTr("Cancel")
                    onClicked: removePopup.close()
                }

                Button {
                    text: qsTr("Remove")
                    onClicked: VPN.removeDevice(removePopup.deviceName)
                }
            }
        }
    }
}
