/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

import org.mozilla.Glean 0.23
import telemetry 0.23

Item {
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

    function closeConnectionInfo() {
        connectionInfo.close();
    }

    state: VPNController.state
    Layout.preferredHeight: 318
    Layout.fillWidth: true
    Layout.leftMargin: 8
    Layout.rightMargin: 8
    Layout.alignment: Qt.AlignHCenter

    Rectangle {
        id: boxBackground
        anchors.fill: box
        color: Theme.bgColor
        radius: 8
        antialiasing: true
    }

    VPNDropShadowWithStates {
        anchors.fill: boxBackground
        source: boxBackground
        cached: true
        transparentBorder: true
        z: -1
    }

    states: [
        State {
            name: VPNController.StateInitializing

            PropertyChanges {
                target: boxBackground
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
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: settingsImage
                source: "qrc:/ui/resources/settings.svg"
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
                target: animatedRings
                visible: false
            }

        },
        State {
            name: VPNController.StateOff

            PropertyChanges {
                target: boxBackground
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
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: settingsImage
                source: "qrc:/ui/resources/settings.svg"
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
                target: animatedRings
                visible: false
            }

        },
        State {
            name: VPNController.StateConnecting

            PropertyChanges {
                target: boxBackground
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
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: settingsImage
                source: "qrc:/ui/resources/settings-white.svg"
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
                target: animatedRings
                visible: false
            }

        },
        State {
            name: VPNController.StateConfirming

            PropertyChanges {
                target: boxBackground
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
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: settingsImage
                source: "qrc:/ui/resources/settings-white.svg"
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
                target: animatedRings
                visible: false
            }

        },
        State {
            name: VPNController.StateOn

            PropertyChanges {
                target: boxBackground
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
                visible: false
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: true
            }

            PropertyChanges {
                target: settingsButton
                buttonColorScheme: Theme.iconButtonDarkBackground
            }

            PropertyChanges {
                target: settingsImage
                source: "qrc:/ui/resources/settings-white.svg"
            }

            PropertyChanges {
                target: connectionInfoButton
                visible: true
            }

            PropertyChanges {
                target: animatedRings
                visible: true
                opacity: 1
                startAnimation: true
            }

        },
        State {
            name: VPNController.StateDisconnecting

            PropertyChanges {
                target: boxBackground
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
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: settingsImage
                source: "qrc:/ui/resources/settings.svg"
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
                target: animatedRings
                visible: false
            }

        },
        State {
            name: VPNController.StateSwitching

            PropertyChanges {
                target: boxBackground
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
                text: qsTrId("vpn.controller.switchingDetail").arg(VPNController.currentLocalizedCityName).arg(VPNController.switchingLocalizedCityName)
                color: "#FFFFFF"
                opacity: 0.8
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: settingsImage
                source: "qrc:/ui/resources/settings-white.svg"
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
                target: animatedRings
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
                target: boxBackground
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
                target: boxBackground
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

    VPNAnimatedRingsShader {
        id: animatedRings
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
            Sample.connectionInfoOpened.record();
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
        enabled: !connectionInfoVisible

        VPNIcon {
            id: connectionInfoImage

            source: "qrc:/ui/resources/connection-info.svg"
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
            Sample.settingsViewOpened.record();
            stackview.push("qrc:/ui/views/ViewSettings.qml", StackView.Immediate)
        }

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: Theme.windowMargin / 2
        anchors.rightMargin: Theme.windowMargin / 2
        //% "Settings"
        accessibleName: qsTrId("vpn.main.settings")
        Accessible.ignored: connectionInfoVisible
        enabled: !connectionInfoVisible

        VPNIcon {
            id: settingsImage

            anchors.centerIn: settingsButton

            Rectangle {
                id: unseenFeaturesIndicator

                states: [
                    State {
                        when: settingsButton.state === Theme.uiState.stateHovered

                        PropertyChanges {
                            target: unseenFeaturesIndicator
                            border.color: settingsButton.buttonColorScheme.buttonHovered
                        }
                    },
                    State {
                        when: settingsButton.state === Theme.uiState.statePressed

                        PropertyChanges {
                            target: unseenFeaturesIndicator
                            border.color: settingsButton.buttonColorScheme.buttonPressed
                        }
                    }
                ]

                transitions: [
                    Transition {
                        ColorAnimation {
                            target: unseenFeaturesIndicator
                            duration: 200
                        }
                    }
                ]

                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 1
                anchors.rightMargin: 1
                border {
                    color: boxBackground.color
                    width: 1
                }
                color: Color.error.default
                height: Theme.listSpacing + border.width * 2
                radius: width / 2
                visible: VPNWhatsNewModel.hasUnseenFeature
                width: Theme.listSpacing + border.width * 2
            }
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
                stackview.push("qrc:/ui/views/ViewSettings.qml", StackView.Immediate);
            }
            function onAboutNeeded() {
                while(stackview.depth > 1) {
                    stackview.pop(null, StackView.Immediate);
                }
                stackview.push(aboutUsComponent);
            }
        }
    }

    Column {
        id: col

        spacing: 0
        anchors.left: box.left
        anchors.right: box.right
        anchors.top: logo.bottom
        anchors.topMargin: 24
        width: parent.width

        function handleMultilineText() {

            const titleIsWrapped = logoTitle.lineCount > 1;
            const subTitleIsWrapped = (logoSubtitle.visible && logoSubtitle.lineCount > 1 || (connectionStability.visible && connectionStability.gridFlow === Grid.TopToBottom));

            if (titleIsWrapped && subTitleIsWrapped) {
                col.anchors.topMargin = 12
                spacer.height = 6;
                return;
            }

            if (subTitleIsWrapped) {
                col.anchors.topMargin = 24
                spacer.height = 4;
                return;
            }

            if (titleIsWrapped) {
                col.anchors.topMargin = 16
                spacer.height = 12;
                return;
            }
            col.anchors.topMargin = 24
            spacer.height = 16;
        }

        VPNHeadline {
            id: logoTitle
            objectName: "controllerTitle"
            lineHeight: 22
            font.pixelSize: 22
            Accessible.ignored: connectionInfoVisible
            Accessible.description: logoSubtitle.text
            width: parent.width
            onPaintedHeightChanged: if (visible) col.handleMultilineText()
        }

        VPNVerticalSpacer {
            id: spacer
            height: 16
        }

        VPNInterLabel {
            id: logoSubtitle
            objectName: "controllerSubTitle"

            lineHeight: Theme.controllerInterLineHeight
            Accessible.ignored: true
            width: parent.width - Theme.windowMargin
            anchors.horizontalCenter: parent.horizontalCenter
            onPaintedHeightChanged: if (visible) col.handleMultilineText()
            onVisibleChanged: if (visible) col.handleMultilineText()
        }

        RowLayout {
          id: logoSubtitleOn

          anchors.horizontalCenter: parent.horizontalCenter
          opacity: 0.8

          VPNInterLabel {
            objectName: "secureAndPrivateSubtitle"

            color: Theme.white
            lineHeight: Theme.controllerInterLineHeight
            Accessible.ignored: true

            //% "Secure and private"
            //: This refers to the user’s internet connection.
            text: qsTrId("vpn.controller.active") + " • "
          }

          VPNSemiMonoLabel {
            id: connectionTime
            Accessible.ignored: true
          }
        }

        VPNConnectionStability {
            id: connectionStability
            Accessible.ignored: connectionInfoVisible || !visible
            width: parent.width
            implicitHeight: childrenRect.height
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
        enabled: !connectionInfoVisible
    }

    VPNConnectionInfo {
        id: connectionInfo
        visible: false

        Behavior on opacity {
            NumberAnimation {
                target: connectionInfo
                property: "opacity"
                duration: 200
            }

        }

    }

}
