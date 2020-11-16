/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Rectangle {
    id: box

    function formatSingle(value) {
        if (value === 0)
            return "00";

        return (value < 10 ? "0" : "") + value;
    }

    function formatTime(time) {
        var secs = time % 60;
        time = Math.floor(time / 60);
        var mins = time % 60;
        time = Math.floor(time / 60);
        return formatSingle(time) + ":" + formatSingle(mins) + ":" + formatSingle(secs);
    }

    state: VPNController.state
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.margins: 16
    radius: 8
    height: 318
    width: parent.width - 32
    antialiasing: true
    states: [
        State {
            name: VPNController.StateInitializing

            PropertyChanges {
                target: box
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoTitle
                //% "VPN is off"
                text: qsTrId("vpn.controller.deactivated")
                color: Theme.fontColorDark
            }

            PropertyChanges {
                target: logoSubtitle
                //% "Turn on to protect your privacy"
                text: qsTrId("vpn.controller.activationSloagan")
                color: Theme.fontColor
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
                target: connectionInfo
                visible: false
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRingsWrapper
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
                target: logoTitle
                text: qsTrId("vpn.controller.deactivated")
                color: Theme.fontColorDark
            }

            PropertyChanges {
                target: logoSubtitle
                text: qsTrId("vpn.controller.activationSloagan")
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
                target: connectionInfo
                visible: false
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRingsWrapper
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
                target: logoTitle
                //% "Connecting"
                text: qsTrId("vpn.controller.connecting") + " …"
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoSubtitle
                //% "Masking connection and location"
                text: qsTrId("vpn.controller.activating")
                color: "#FFFFFF"
                opacity: 0.8
            }

            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
            }

            PropertyChanges {
                target: settingsButton
                buttonColorScheme: Theme.iconButtonDarkBackground
            }

            PropertyChanges {
                target: connectionInfoButton
                visible: false
            }

            PropertyChanges {
                target: connectionInfo
                visible: false
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRingsWrapper
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
                target: logoTitle
                //% "VPN is on"
                text: qsTrId("vpn.controller.activated")
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoSubtitle
                //% "Secure and private"
                //: This refers to the user’s internet connection.
                text: qsTrId("vpn.controller.active")+ "  •  " + formatTime(VPNController.time)
                visible: VPNConnectionHealth.stability === VPNConnectionHealth.Stable
                color: "#FFFFFF"
                opacity: 0.8
            }

            PropertyChanges {
                target: settingsButton
                buttonColorScheme: Theme.iconButtonDarkBackground
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
                target: logoTitle
                //% "Disconnecting…"
                text: qsTrId("vpn.controller.disconnecting")
                color: Theme.fontColorDark
            }

            PropertyChanges {
                target: logoSubtitle
                //% "Unmasking connection and location"
                text: qsTrId("vpn.controller.deactivating")
                color: Theme.fontColor
                opacity: 1
            }

            PropertyChanges {
                target: settingsImage
                source: "../resources/settings.svg"
            }

            PropertyChanges {
                target: settingsButton
                buttonColorScheme: Theme.iconButtonLightBackground
            }

            PropertyChanges {
                target: connectionInfoButton
                visible: false
            }

            PropertyChanges {
                target: connectionInfo
                visible: false
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRingsWrapper
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
                target: logoTitle
                //% "Switching…"
                text: qsTrId("vpn.controller.switching")
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoSubtitle
                //% "From %1 to %2"
                //: Switches from location 1 to location 2
                text: qsTrId("vpn.controller.switchingDetail").arg(VPNController.currentCity).arg(VPNController.switchingCity)
                color: "#FFFFFF"
                opacity: 0.8
            }

            PropertyChanges {
                target: settingsImage
                source: "../resources/settings-white.svg"
            }

            PropertyChanges {
                target: settingsButton
                buttonColorScheme: Theme.iconButtonDarkBackground
            }

            PropertyChanges {
                target: connectionInfoButton
                visible: true
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRingsWrapper
                visible: false
            }

            PropertyChanges {
                target: animatedRingsWrapper
                visible: true
                opacity: 1
                startAnimation: true
            }

        },
        State {
            name: VPNController.StateCaptivePortal

            PropertyChanges {
                target: box
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoTitle
                text: qsTrId("vpn.controller.deactivated") // TODO
                color: Theme.fontColorDark
                opacity: 0.55
            }

            PropertyChanges {
                target: logoSubtitle
                text: qsTrId("vpn.controller.activationSloagan") // TODO
                color: Theme.fontColor
                opacity: 0.55
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
                target: connectionInfo
                visible: false
            }

            PropertyChanges {
                target: connectionStability
                visible: false
            }

            PropertyChanges {
                target: animatedRingsWrapper
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

    VPNAnimatedRings {
        id: animatedRingsWrapper
    }

    VPNMainImage {
        id: logo

        anchors.horizontalCenter: parent.horizontalCenter
        y: 50
        height: 80
        width: 80
    }

    VPNIconButton {
        id: connectionInfoButton

        onClicked: connectionInfo.visible = true
        buttonColorScheme: Theme.iconButtonDarkBackground
        opacity: connectionInfoButton.visible ? 1 : 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        //% "Connection Information"
        accessibleName: qsTrId("vpn.controller.info")

        VPNIcon {
            id: connectionInfoImage

            source: "../resources/connection-info.svg"
            anchors.centerIn: connectionInfoButton
            sourceSize.height: 20
            sourceSize.width: 20
            visible: connectionInfoButton.visible
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 300
            }

        }

    }

    VPNIconButton {
        id: settingsButton

        onClicked: stackview.push("../views/ViewSettings.qml", StackView.Immediate)
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: Theme.windowMargin / 2
        anchors.rightMargin: Theme.windowMargin / 2
        //% "Settings"
        accessibleName: qsTrId("vpn.main.settings")

        VPNIcon {
            id: settingsImage

            anchors.centerIn: settingsButton
        }

        Component {
            id: aboutUsComponent

            VPNAboutUs {
                isSettingsView: false
            }

        }
        Connections {
            target: VPN
            function onSettingsNeeded() {
                while(stackview.depth > 1) {
                    stackview.pop(null, StackView.Immediate);
                }
                stackview.push("../views/ViewSettings.qml", StackView.Immediate);
            }
            function onAboutNeeded() {
                while(stackview.depth > 1) {
                    stackview.pop(null, StackView.Immediate);
                }
                stackview.push(aboutUsComponent);
            }
        }
    }

    VPNHeadline {
        id: logoTitle

        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logo.bottom
        anchors.topMargin: 24
        font.pixelSize: 22
    }

    VPNInterLabel {
        id: logoSubtitle

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logoTitle.bottom
        anchors.topMargin: Theme.windowMargin / 2
        lineHeight: Theme.controllerInterLineHeight
        width: box.width - Theme.windowMargin * 3
    }

    VPNConnectionStability {
        id: connectionStability

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logoTitle.bottom
        anchors.topMargin: Theme.windowMargin / 2
        visible: false
    }

    VPNToggle {
        id: toggle

        anchors.bottom: box.bottom
        anchors.bottomMargin: 48
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
    }

    VPNConnectionInfo {
        id: connectionInfo

        anchors.fill: parent
        height: box.height
        width: box.width
        visible: false
        opacity: connectionInfo.visible ? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                target: connectionInfo
                property: "opacity"
                duration: 200
            }

        }

    }

}
