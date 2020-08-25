import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"

Item {
    VPNControllerView {
        id: box
    }

    DropShadow {
        anchors.fill: box
        horizontalOffset: 0
        verticalOffset: 1
        radius: 4
        color: "#0C0C0D1E"
        source: box
    }

    RowLayout {
        width: parent.width
        y: box.y + box.height + 28

        Image {
            width: 16
            height: 16
            source: "../resources/settings.svg"
            sourceSize.width: 16
            sourceSize.height: 16
            Layout.leftMargin: 20
        }

        Label {
            text: qsTr("Select location")
            color: "#3D3D3D"
            font.family: vpnFont.name
            font.pixelSize: 15
            font.weight: Font.Bold
        }

        Item {
            Layout.fillWidth: true
        }

        Image {
            source: "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase() + ".png"
            sourceSize.height: 16
            sourceSize.width: 16
        }

        Label {
            text: VPNCurrentServer.city
            color: "#3D3D3D"
            font.family: vpnFont.name
            font.pixelSize: 15

            MouseArea {
                anchors.fill: parent
                onClicked: stackview.push("ViewServers.qml")
            }
        }

        Image {
            width: 16
            height: 16
            source: "../resources/chevron.svg"
            sourceSize.width: 16
            sourceSize.height: 16
            Layout.rightMargin: 20

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    // TODO: connecting/disconnectin we should gray it out.
                    if (VPNController.state === VPNController.StateOn ||
                            VPNController.state === VPNController.StateOff) {
                        stackview.push("ViewServers.qml")
                    }
                }
            }
        }
    }

    RowLayout {
        width: parent.width
        y: box.y + box.height + 74

        Image {
            width: 16
            height: 16
            source: "../resources/devices.svg"
            sourceSize.width: 16
            sourceSize.height: 16
            Layout.leftMargin: 20
        }

        Label {
            text: qsTr("My devices")
            color: "#3D3D3D"
            font.family: vpnFont.name
            font.pixelSize: 15
            font.weight: Font.Bold
        }

        Item {
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("%1 of %2").arg(VPNDeviceModel.activeDevices).arg(VPNUser.maxDevices)
            color: "#3D3D3D"
            font.family: vpnFont.name
            font.pixelSize: 15

            MouseArea {
                anchors.fill: parent
                onClicked: stackview.push("ViewDevices.qml")
            }
        }

        Image {
            width: 16
            height: 16
            source: "../resources/chevron.svg"
            sourceSize.width: 16
            sourceSize.height: 16
            Layout.rightMargin: 20

            MouseArea {
                anchors.fill: parent
                onClicked: stackview.push("ViewDevices.qml")
            }
        }
    }
}
