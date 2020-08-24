import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    VPNMenu {
        id: menu
        title: qsTr("My devices")
        rightTitle: qsTr("%1 of %2").arg(VPNDeviceModel.activeDevices).arg(
                        VPNUser.maxDevices)
    }

    ListView {
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        clip: true

        model: VPNDeviceModel

        delegate: Container {
            id: device
            width: parent.width

            contentItem: ColumnLayout {

                RowLayout {
                    id: deviceRow
                    Layout.topMargin: (Theme.vSpacing / 2)
                    Layout.bottomMargin: deviceRow.Layout.topMargin

                    VPNIcon {
                        id: deviceImage
                        source: "../resources/devices.svg"
                        Layout.leftMargin: Theme.hSpacing
                        Layout.rightMargin: Theme.hSpacing
                        Layout.topMargin: 0
                        Layout.alignment: Qt.AlignTop
                    }

                    ColumnLayout {
                        Label {
                            id: deviceName
                            text: name
                            color: "#3D3D3D"
                            font.pixelSize: 15
                        }

                        Label {
                            function deviceSubtitle() {
                                if (currentOne) {
                                    return qsTr("Current device");
                                }

                                const diff = (Date.now() - createdAt.valueOf()) / 1000;

                                if (diff < 3600) {
                                    return qsTr("Added less than a hour ago");
                                }

                                if (diff < 86400) {
                                    return qsTr("Added %1 hours ago").arg(Math.floor(diff / 3600));
                                }

                                return qsTr("Added %1 days ago").arg(Math.floor(diff / 86400));
                            }

                            id: deviceDesc
                            text: deviceSubtitle()
                            font.pixelSize: 13
                            color: currentOne ? "#0060DF" : "#6D6D6E"
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    VPNIcon {
                        visible: !currentOne
                        id: deleteImage
                        Layout.rightMargin: Theme.hSpacing
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
                text: qsTr("Please confirm you would like to remove\n%1.").arg(removePopup.deviceName)
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
                    onClicked: {
                        VPN.removeDevice(removePopup.deviceName)
                        removePopup.close()
                    }
                }
            }
        }
    }
}
