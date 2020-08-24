import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

Rectangle {
    state: VPNController.state

    states: [
        State {
            name: VPNController.StateOff
            PropertyChanges {
                target: logoTitle
                text: qsTr("VPN is off")
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-off.svg"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Turn on to protect your privacy")
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
            }
        },
        State {
            name: VPNController.StateConnecting
            PropertyChanges {
                target: logoTitle
                text: qsTr("Connecting...")
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-on.svg"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Masking connection and location")
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
            }
        },
        State {
            name: VPNController.StateOn
            PropertyChanges {
                target: logoTitle
                text: qsTr("VPN is on")
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-on.svg"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Secure and private  •  00:04:85")
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/setting-white.svg"
            }
        }
    ]

    transitions: [
        Transition {
            from: VPNController.StateOff
            to: VPNController.StateConnecting
            ColorAnimation {
                target: box
                property: "color"
                to: "#321C64"
                duration: 200
            }
            ColorAnimation {
                target: logoTitle
                property: "color"
                to: "#FFFFFF"
                duration: 200
            }
            ColorAnimation {
                target: logoSubtitle
                property: "color"
                to: "#FFFFFF"
                duration: 200
            }
        },

        Transition {
            from: VPNController.StateConnecting
            to: VPNController.StateOff
            ColorAnimation {
                target: box
                property: "color"
                to: "#FFFFFF"
                duration: 200
            }
            ColorAnimation {
                target: logoTitle
                property: "color"
                to: "#3D3D3D"
                duration: 200
            }
            ColorAnimation {
                target: logoSubtitle
                property: "color"
                to: "#3D3D3D"
                duration: 200
            }
        }
    ]

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

        MouseArea {
            anchors.fill: parent
            onClicked: stackview.push("../views/ViewSettings.qml")
        }
    }

    Image {
        id: logo
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: 66
        sourceSize.width: 76
        sourceSize.height: 76
    }

    Text {
        id: logoTitle
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter

        font.family: vpnFont.name
        horizontalAlignment: Text.AlignHCenter
        y: logo.y + logo.height + 26
        font.pixelSize: 22
        height: 32
        color: "#3D3D3D"
    }

    Text {
        id: logoSubtitle
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: logoTitle.y + logoTitle.height + 8
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 15
        height: 22
        color: "#3D3D3D"
    }

    VPNToggle {
        id: toggle
        y: logoSubtitle.y + logoSubtitle.height + 24
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
