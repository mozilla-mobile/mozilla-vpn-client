import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

Rectangle {
    state: VPNController.state

    function formatSingle(value) {
        if (value === 0) return "00";
        return (value < 10 ? "0" : "") + value;
    }

    function formatTime(time) {
        var secs = time % 60;
        time = Math.floor(time / 60);

        var mins = time % 60;
        time = Math.floor(time / 60);

        return formatSingle(time) + ":" + formatSingle(mins) + ":" + formatSingle(secs);
    }

    states: [
        State {
            name: VPNController.StateInitializing
            PropertyChanges {
                target: box
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-off.svg"
            }
            PropertyChanges {
                target: logoTitle
                text: qsTr("VPN is off")
                color: Theme.fontColorDark
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Turn on to protect your privacy")
                color: Theme.fontColor
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
            }
            PropertyChanges {
                target: connectionStability
                visible: false
            }
            PropertyChanges {
                target: animation
                visible: false
            }
        },
        State {
            name: VPNController.StateOff
            PropertyChanges {
                target: box
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-off.svg"
            }
            PropertyChanges {
                target: logoTitle
                text: qsTr("VPN is off")
                color: Theme.fontColorDark
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Turn on to protect your privacy")
                color: Theme.fontColor
                opacity: 1
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
            }
            PropertyChanges {
                target: connectionInfoButton
                visible: false
            }
            PropertyChanges {
                target: connectionStability
                visible: false
            }
        },
        State {
            name: VPNController.StateConnecting
            PropertyChanges {
                target: box
                color: "#321C64"
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-on.svg"
            }
            PropertyChanges {
                target: logoTitle
                text: qsTr("Connecting…")
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Masking connection and location")
                color: "#FFFFFF"
                opacity: .8
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
            }
            PropertyChanges {
                target: settingsButton
                backgroundColor: Theme.whiteSettingsBtn
            }
            PropertyChanges {
                target: connectionInfoButton
                visible: false
            }
            PropertyChanges {
                target: connectionStability
                visible: false
            }
        },
        State {
            name: VPNController.StateOn
            PropertyChanges {
                target: box
                color: "#321C64"
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-on.svg"
            }
            PropertyChanges {
                target: logoTitle
                text: qsTr("VPN is on")
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Secure and private  •  ") + formatTime(VPNController.time)
                visible: VPNConnectionHealth.stability === VPNConnectionHealth.Stable
                color: "#FFFFFF"
                opacity: .8
            }
            PropertyChanges {
                target: settingsButton
                backgroundColor: Theme.whiteSettingsBtn
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
            }
            PropertyChanges {
                target: connectionInfoButton
                visible: true
            }
            PropertyChanges {
                target: animatedRingsWrapper
                visible: true
                opacity: 1
                startAnimation: true
            }
        },
        State {
            name: VPNController.StateDisconnecting
            PropertyChanges {
                target: box
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-off.svg"
            }
            PropertyChanges {
                target: logoTitle
                text: qsTr("Disconnecting…")
                color: Theme.fontColorDark
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Unmasking connection and location")
                color: Theme.fontColor
                opacity: 1
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
            }
            PropertyChanges {
                target: connectionInfoButton
                visible: false
            }
            PropertyChanges {
                target: connectionStability
                visible: false
            }
        },
        State {
            name: VPNController.StateSwitching
            PropertyChanges {
                target: box
                color: "#321C64"
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-on.svg"
            }
            PropertyChanges {
                target: logoTitle
                text: qsTr("Switching…")
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("From %1 to %2").arg(VPNController.currentCity).arg(VPNController.switchingCity)
                color: "#FFFFFF"
                opacity: .8
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
            }
            PropertyChanges {
                target: connectionInfoButton
                visible: true
            }
            PropertyChanges {
                target: connectionInfoButton
                visible: true
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }
        },
        State {
            name: VPNController.StateDeviceLimit
            PropertyChanges {
                target: box
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logo
                source: "../resources/state-off.svg"
                opacity: .55
            }
            PropertyChanges {
                target: logoTitle
                text: qsTr("VPN is off")
                color: Theme.fontColorDark
                opacity: .55
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Turn on to protect your privacy")
                color: Theme.fontColor
                opacity: .55
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
            }
            PropertyChanges {
                target: connectionStability
                visible: false
            }
        }
    ]

    transitions: [
        Transition {
            to: VPNController.StateConnecting
            ColorAnimation {
                target: box
                property: "color"
                duration: 200
            }
            ColorAnimation {
                target: logoTitle
                property: "color"
                duration: 200
            }
            ColorAnimation {
                target: logoSubtitle
                property: "color"
                duration: 200
            }
        },

        Transition {
            to: VPNController.StateDisconnecting
            ColorAnimation {
                target: box
                property: "color"
                duration: 200
            }
            ColorAnimation {
                target: logoTitle
                property: "color"
                duration: 200
            }
            ColorAnimation {
                target: logoSubtitle
                property: "color"
                duration: 200
            }
        }
    ]

    id: box
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.margins: 16
    radius: 8
    height: 318
    width: parent.width - 32
    antialiasing: true

    VPNAnimatedRings {
        id: animatedRingsWrapper
    }

    VPNIconButton {
        id: connectionInfoButton
        onClicked: connectionInfo.visible = true
        defaultColor: box.color
        backgroundColor: Theme.whiteSettingsBtn
        opacity: connectionInfoButton.visible? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: 300
            }
        }
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2

        Image {
            id: connectionInfoImage
            source: "../resources/connection-graph.svg";
            anchors.centerIn :connectionInfoButton
            sourceSize.height: 22
            sourceSize.width: 22
        }
    }

    VPNIconButton {
        id: settingsButton
        onClicked: stackview.push("../views/ViewSettings.qml")
        defaultColor: box.color
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: Theme.windowMargin / 2
        anchors.rightMargin: Theme.windowMargin / 2

        Image {
            id: settingsImage
            anchors.centerIn :settingsButton
            sourceSize.height: 22
            sourceSize.width: 22
        }
    }

    Image {
        id: logo
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: 50
        sourceSize.width: 80
        sourceSize.height: 80
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
    }

    VPNInterLabel {
        id: logoSubtitle
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: logoTitle.y + logoTitle.height + 8
    }

    VPNConnectionStability {
        id: connectionStability
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: logoTitle.y + logoTitle.height + 8
        visible: false
    }

    VPNToggle {
        id: toggle
        y: logoSubtitle.y + logoSubtitle.height + 24
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
    }

    VPNConnectionInfo {
        id: connectionInfo
        anchors.fill: parent
        height: box.height
        width: box.width
        visible: false
        opacity: connectionInfo.visible? 1 : 0


        Behavior on opacity {
            NumberAnimation {
                target: connectionInfo
                property: "opacity"
                duration: 200
            }
        }

    }
}
