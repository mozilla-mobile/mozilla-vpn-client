import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

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
        id: devicesRow1
        spacing: 0
        width: parent.width
        height: Theme.vSpacing
        y: box.y + box.height + Theme.iconSize

        VPNIcon {
            source: "../resources/connection.svg"
            Layout.rightMargin: Theme.iconSize
            Layout.leftMargin: Theme.hSpacing
        }

        VPNBoldLabel {
            text: qsTr("Select location")
        }

        Item {
            Layout.fillWidth: true
        }

        VPNIcon {
            Layout.rightMargin: 8
            source: "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase(
                        ) + ".png"
        }

        VPNLightLabel {
            text: VPNCurrentServer.city
            MouseArea {
                anchors.fill: parent
                onClicked: stackview.push("ViewServers.qml")
            }
        }

        VPNChevron {
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
        spacing: 0
        width: parent.width
        y: box.y + box.height + Theme.iconSize + (Theme.vSpacing * 2)

        VPNIcon {
            source: "../resources/devices.svg"
            Layout.rightMargin: Theme.iconSize
            Layout.leftMargin: Theme.hSpacing
        }

        VPNBoldLabel {
            text: qsTr("My devices")
        }

        Item {
            Layout.fillWidth: true
        }

        VPNLightLabel {
            text: qsTr("%1 of %2").arg(VPNDeviceModel.activeDevices).arg(
                      VPNUser.maxDevices)
            MouseArea {
                anchors.fill: parent
                onClicked: stackview.push("ViewDevices.qml")
            }
        }

        VPNChevron {
            MouseArea {
                anchors.fill: parent
                onClicked: stackview.push("ViewDevices.qml")
            }
        }
    }
}
