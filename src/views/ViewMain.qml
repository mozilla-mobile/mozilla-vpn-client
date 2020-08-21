import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
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

    RoundButton {
        x: 130
        y: 347
        width: 282
        height: 40
        text: qsTr("Devices") + " "+ VPN.activeDevices + "/"+ VPN.user.maxDevices
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        font.weight: Font.ExtraLight
        enabled: true
        focusPolicy: Qt.NoFocus
        radius: 5
        onClicked: stackview.push("ViewDevices.qml")
    }

    RoundButton {
        x: 130
        y: 400
        width: 282
        height: 40
        text: qsTr("Servers")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        font.weight: Font.ExtraLight
        enabled: true
        focusPolicy: Qt.NoFocus
        radius: 5
        onClicked: stackview.push("ViewServers.qml")
    }
}
