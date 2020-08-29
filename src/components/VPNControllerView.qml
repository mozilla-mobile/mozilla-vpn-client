import QtQuick 2.0
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
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
            }
            PropertyChanges {
                target: logoTitle
                color: Theme.fontColorDark
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Turn on to protect your privacy")
            }
            PropertyChanges {
                target: logoSubtitle
                color: Theme.fontColor
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
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
            }
            PropertyChanges {
                target: logoTitle
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Masking connection and location")
            }
            PropertyChanges {
                target: logoSubtitle
                color: "#FFFFFFCC"
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
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
            }
            PropertyChanges {
                target: logoTitle
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Secure and private  •  ") + formatTime(VPNController.time)
            }
            PropertyChanges {
                target: logoSubtitle
                color: "#FFFFFFCC"
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
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
            }
            PropertyChanges {
                target: logoTitle
                color: Theme.fontColorDark
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("Unmasking connection and location")
            }
            PropertyChanges {
                target: logoSubtitle
                color: Theme.fontColor
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
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
            }
            PropertyChanges {
                target: logoTitle
                color: "#FFFFFF"
            }
            PropertyChanges {
                target: logoSubtitle
                text: qsTr("From %1 to %2").arg(VPNController.currentCity).arg(VPNController.switchingCity)
            }
            PropertyChanges {
                target: logoSubtitle
                color: "#FFFFFFCC"
            }
            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
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
        },

        Transition {
            from: VPNController.StateConnecting
            to: VPNController.StateOn
            ScriptAction {
                script: if (vpnSystemTray.supportsMessages) {
                            // TODO: message, icon and time have to be defined.
                            vpnSystemTray.showMessage(qsTr("Mozilla VPN connected"), vpnSystemTray.NoIcon, 2000);
                        }
            }
        },

        Transition {
            from: VPNController.StateDisconnecting
            to: VPNController.StateOff
            ScriptAction {
                script: if (vpnSystemTray.supportsMessages) {
                            // TODO: message, icon and time have to be defined.
                            vpnSystemTray.showMessage(qsTr("Mozilla VPN disconnected"), vpnSystemTray.NoIcon, 2000);
                        }
            }
        },

        Transition {
            from: VPNController.StateSwitching
            to: VPNController.StateOn
            ScriptAction {
                script: if (vpnSystemTray.supportsMessages) {
                            // TODO: message, icon and time have to be defined.
                            vpnSystemTray.showMessage(qsTr("Mozilla VPN switched"), vpnSystemTray.NoIcon, 2000);
                        }
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

    Image {
        id: settingsImage
        height: 16
        width: 16
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 16
        anchors.rightMargin: 16
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            onClicked: stackview.push("../views/ViewSettings.qml")
        }
    }

    Image {
        id: logo
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.y + 50
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
    }

    Text {
        id: logoSubtitle
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        y: logoTitle.y + logoTitle.height + 8
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 15
        height: 22
    }

    VPNToggle {
        id: toggle
        y: logoSubtitle.y + logoSubtitle.height + 24
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
