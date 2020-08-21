import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"

Item {
    Rectangle {
        id: box
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        color: "#FFFFFF"
        radius: 8
        height: 328
        width: parent.width - 32

        Image {
            id: settingsImage
            height: 16
            width: 16
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 16
            anchors.rightMargin: 16
            source: "../resources/settings.svg"

            MouseArea {
                anchors.fill: parent
                onClicked: stackview.push("ViewSettings.qml")
            }
        }

        Image {
            id: logo
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            y: 66
            source: "../resources/state-off.svg"
            sourceSize.width: 76
            sourceSize.height: 76
        }

        Text {
            id: logoTitle
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter

            text: qsTr("VPN is off")
            font.family: vpnFont.name
            horizontalAlignment: Text.AlignHCenter
            y: logo.y + logo.height + 26
            font.pixelSize: 22
            height: 32
        }

        Text {
            id: logoSubtitle
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            y: logoTitle.y + logoTitle.height + 8
            text: qsTr("Turn on to protect your privacy")
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 15
            height: 22
        }

        VPNToggle {
            id: toggle
            y: logoSubtitle.y + logoSubtitle.height + 24
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            onActivate: VPN.activate()
            onDeactivate: VPN.deactivate()
        }
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
            source: "../resources/flags/" + VPN.currentServer.countryCode.toUpperCase() + ".png"
            sourceSize.height: 16
            sourceSize.width: 16
        }

        Label {
            text: VPN.currentServer.city
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
                onClicked: stackview.push("ViewServers.qml")
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
            text: qsTr("%1 of %2").arg(VPN.activeDevices).arg(VPN.user.maxDevices)
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
