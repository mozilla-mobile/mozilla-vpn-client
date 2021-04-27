/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme
import "/glean/load.js" as Glean

Rectangle {
    id: box

    readonly property alias connectionInfoVisible: connectionInfo.visible

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
                //% "Connecting…"
                text: qsTrId("vpn.controller.connectingState")
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
            name: VPNController.StateConfirming

            PropertyChanges {
                target: box
                color: "#321C64"
            }

            PropertyChanges {
                target: logoTitle
                text: qsTrId("vpn.controller.connectingState")
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoSubtitle
                text: VPNController.connectionRetry > 1 ?
                          //% "Attempting to confirm connection"
                          qsTrId("vpn.controller.attemptingToConfirm") :
                          qsTrId("vpn.controller.activating")
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
                text: qsTrId("vpn.controller.active") + "  •  " + formatTime(VPNController.time)
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
                opacity: 1
                startAnimation: false
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
        // Make sure we only do the render animation when
        // The element is visible &&
        // the application is not minimized
        isCurrentyVisible: stackview.depth === 1 &&
                           (Qt.application.state === Qt.ApplicationActive ||
                            Qt.application.state === Qt.ApplicationInactive)
    }

    VPNMainImage {
        id: logo

        anchors.horizontalCenter: parent.horizontalCenter
        y: 48
        height: 80
        width: 80
    }

    VPNIconButton {
        id: connectionInfoButton
        objectName: "connectionInfoButton"

        onClicked: {
            Glean.sample.connectionInfoOpened.record();
            connectionInfo.open()
        }

        buttonColorScheme: Theme.iconButtonDarkBackground
        opacity: connectionInfoButton.visible ? 1 : 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        //% "Connection Information"
        accessibleName: qsTrId("vpn.controller.info")
        Accessible.ignored: connectionInfoVisible

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
        objectName: "settingsButton"
        opacity: 1

        onClicked: {
            Glean.sample.settingsViewOpened.record();
            stackview.push("../views/ViewSettings.qml", StackView.Immediate)
        }

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: Theme.windowMargin / 2
        anchors.rightMargin: Theme.windowMargin / 2
        //% "Settings"
        accessibleName: qsTrId("vpn.main.settings")
        Accessible.ignored: connectionInfoVisible

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

    ColumnLayout {
        id: col

        spacing: 0
        width: box.width - Theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter

        function handleMultilineText() {
            const titleIsWrapped = logoTitle.lineCount > 1;
            const subTitleIsWrapped = (logoSubtitle.lineCount > 1 || (connectionStability.visible && connectionStability.numGridColumns === 1));

            if (titleIsWrapped && subTitleIsWrapped) {
                topTextMargin.Layout.preferredHeight = topTextMargin._preferredHeight - 12
                bottomTextMargin.Layout.preferredHeight = 6;
                return;
            }

            if (subTitleIsWrapped) {
                topTextMargin.Layout.preferredHeight = topTextMargin._preferredHeight - 6
                bottomTextMargin.Layout.preferredHeight = 2;
                return;
            }

            if (titleIsWrapped) {
                topTextMargin.Layout.preferredHeight = topTextMargin._preferredHeight - 4
                bottomTextMargin.Layout.preferredHeight = 8;
                return;
            }
            bottomTextMargin.Layout.preferredHeight = 8;
            topTextMargin.Layout.preferredHeight = topTextMargin._preferredHeight;
        }

        VPNVerticalSpacer {
            property var _preferredHeight: logo.y + logo.height + 24

            id: topTextMargin
            Layout.preferredHeight: _preferredHeight
        }

        ColumnLayout {
            Layout.minimumHeight: 32
            Layout.fillWidth: true
            spacing: 0

            VPNHeadline {
                id: logoTitle
                objectName: "controllerTitle"

                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.minimumHeight: 32
                lineHeight: 22
                font.pixelSize: 22
                Accessible.ignored: connectionInfoVisible
                Accessible.description: logoSubtitle.text
                onPaintedHeightChanged: col.handleMultilineText()
            }
        }

        VPNVerticalSpacer {
            id: bottomTextMargin
            Layout.preferredHeight: 8
            Layout.fillWidth: true
        }

        VPNInterLabel {
            id: logoSubtitle
            objectName: "controllerSubTitle"

            lineHeight: Theme.controllerInterLineHeight
            Layout.preferredWidth: parent.width
            Accessible.ignored: true
            onPaintedHeightChanged: col.handleMultilineText()
        }

        VPNConnectionStability {
            id: connectionStability
            visible: false
            Accessible.ignored: connectionInfoVisible || !visible
        }

    }

    VPNToggle {
        id: toggle
        objectName: "controllerToggle"

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 48
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        Accessible.ignored: connectionInfoVisible
    }

    VPNConnectionInfo {
        id: connectionInfo

        Behavior on opacity {
            NumberAnimation {
                target: connectionInfo
                property: "opacity"
                duration: 200
            }

        }

    }

}
