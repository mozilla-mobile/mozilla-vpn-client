/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0


Item {
    id: box

    property bool connectionInfoScreenVisible: false

    function closeConnectionInfo() {
        box.connectionInfoScreenVisible = false;
    }

    Layout.preferredHeight: 318
    Layout.fillWidth: true
    Layout.leftMargin: VPNTheme.theme.listSpacing
    Layout.rightMargin: VPNTheme.theme.listSpacing
    Layout.alignment: Qt.AlignHCenter

    Behavior on Layout.preferredWidth  {
        NumberAnimation {
            duration: 1000
            easing.type: Easing.InOutQuad
        }
    }

    Rectangle {
        id: boxBackground

        property var maximumBoxHeight: window.safeContentHeight - VPNTheme.theme.windowMargin * 2 - (window.fullscreenRequired() ? box.y + VPNTheme.theme.windowMargin : 0);
        color: VPNTheme.theme.transparent

        radius: VPNTheme.theme.cornerRadius * 2
        antialiasing: true

        height: box.connectionInfoScreenVisible
            ? maximumBoxHeight
            : box.height
        width: box.width

        Behavior on height {
            NumberAnimation {
                duration: connectionInfoScreen.transitionDuration
                easing.type: Easing.InOutQuad
            }
        }
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
            name: "stateInitializing"
            when: VPNController.state === VPNController.StateInitializing

            PropertyChanges {
                target: boxBackground
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoTitle
                //% "VPN is off"
                text: qsTrId("vpn.controller.deactivated")
                color: VPNTheme.theme.fontColorDark
            }

            PropertyChanges {
                target: logoSubtitle
                //% "Turn on to protect your privacy"
                text: qsTrId("vpn.controller.activationSloagan")
                color: VPNTheme.theme.fontColor
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: connectionInfoToggleButton
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
            name: "stateOff"
            when: VPNController.state === VPNController.StateOff

            PropertyChanges {
                target: boxBackground
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoTitle
                text: qsTrId("vpn.controller.deactivated")
                color: VPNTheme.theme.fontColorDark
            }

            PropertyChanges {
                target: logoSubtitle
                text: qsTrId("vpn.controller.activationSloagan")
                color: VPNTheme.theme.fontColor
                opacity: 1
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: connectionInfoToggleButton
                visible: connectionInfoScreenVisible
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
            name: "stateConnecting"
            when: VPNController.state === VPNController.StateConnecting

            PropertyChanges {
                target: boxBackground
                color: VPNTheme.colors.primary
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
                target: connectionInfoToggleButton
                visible: connectionInfoScreenVisible
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
            name: "stateConfirming"
            when: VPNController.state === VPNController.StateConfirming

            PropertyChanges {
                target: boxBackground
                color: VPNTheme.colors.primary
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
                target: connectionInfoToggleButton
                visible: connectionInfoScreenVisible
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
            name: "stateOn"
            when: (VPNController.state === VPNController.StateOn ||
                   VPNController.state === VPNController.StateSilentSwitching)

            PropertyChanges {
                target: boxBackground
                color: VPNTheme.colors.primary
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
                target: connectionInfoToggleButton
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
            name: "stateDisconnecting"
            when: VPNController.state === VPNController.StateDisconnecting

            PropertyChanges {
                target: boxBackground
                color: "#FFFFFF"
            }

            PropertyChanges {
                target: logoTitle
                //% "Disconnecting…"
                text: qsTrId("vpn.controller.disconnecting")
                color: VPNTheme.theme.fontColorDark
            }

            PropertyChanges {
                target: logoSubtitle
                //% "Unmasking connection and location"
                text: qsTrId("vpn.controller.deactivating")
                color: VPNTheme.theme.fontColor
                opacity: 1
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: connectionInfoToggleButton
                visible: connectionInfoScreenVisible
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
            name: "stateSwitching"
            when: VPNController.state === VPNController.StateSwitching

            PropertyChanges {
                target: boxBackground
                color: VPNTheme.colors.primary
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
                text: qsTrId("vpn.controller.switchingDetail").arg(VPNCurrentServer.localizedPreviousExitCityName).arg(VPNCurrentServer.localizedExitCityName)
                color: "#FFFFFF"
                opacity: 0.8
                visible: true
            }

            PropertyChanges {
                target: logoSubtitleOn
                visible: false
            }

            PropertyChanges {
                target: connectionInfoToggleButton
                visible: connectionInfoScreenVisible
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
            to: "stateConnecting"

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
            to: "stateDisconnecting"

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

    VPNAnimatedRings {
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
        id: connectionInfoToggleButton
        objectName: "connectionInfoToggleButton"

        //% "Close"
        property var connectionInfoCloseText: qsTrId("vpn.connectionInfo.close")

        anchors {
            left: parent.left
            leftMargin: VPNTheme.theme.windowMargin / 2
            top: parent.top
            topMargin: VPNTheme.theme.windowMargin / 2
        }
        accessibleName: box.connectionInfoScreenVisible ? connectionInfoCloseText : VPNI18n.ConnectionInfoOpenButton
        Accessible.ignored: !visible
        buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
        enabled: visible && !ipInfoPanel.isOpen
        opacity: visible ? 1 : 0
        z: 1

        onClicked: {
            if (!box.connectionInfoScreenVisible) {
                MZGleanDeprecated.recordGleanEvent("connectionInfoOpened");
                Glean.sample.connectionInfoOpened.record();
            } else if (VPNConnectionBenchmark.state === VPNConnectionBenchmark.StateRunning) {
                MZGleanDeprecated.recordGleanEventWithExtraKeys("speedTestCompleted", { "speed": "Cancelled" });
                Glean.sample.speedTestCompleted.record({ speed: "Cancelled" });
            }
            box.connectionInfoScreenVisible = !box.connectionInfoScreenVisible;
        }

        Image {
            property int iconSize: box.connectionInfoScreenVisible
                ? VPNTheme.theme.iconSize
                : VPNTheme.theme.iconSize * 1.5

            anchors.centerIn: connectionInfoToggleButton
            source: box.connectionInfoScreenVisible
                ? "qrc:/nebula/resources/close-white.svg"
                : "qrc:/nebula/resources/bandwidth.svg"
            sourceSize.height: iconSize
            sourceSize.width: iconSize
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 300
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
            Accessible.ignored: connectionInfoScreenVisible
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

            lineHeight: VPNTheme.theme.controllerInterLineHeight
            Accessible.ignored: true
            width: parent.width - VPNTheme.theme.windowMargin
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

            color: VPNTheme.theme.white
            lineHeight: VPNTheme.theme.controllerInterLineHeight
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
            Accessible.ignored: connectionInfoScreenVisible || !visible
            width: parent.width
            implicitHeight: childrenRect.height
        }

    }

    VPNToggle {
        id: toggle
        objectName: "controllerToggle"

        anchors {
            bottom: parent.bottom
            bottomMargin: 48
            horizontalCenterOffset: 0
            horizontalCenter: parent.horizontalCenter
        }
        enabled: !connectionInfoScreenVisible && !ipInfoPanel.visible

        Accessible.ignored: connectionInfoScreenVisible || ipInfoPanel.isOpen
    }

    VPNIPInfoPanel {
        id: ipInfoPanel
        objectName: "ipInfoPanel"

        opacity: ipInfoPanel.isOpen ? 1 : 0
        visible: opacity > 0
        z: 1

        Connections {
            target: VPNConnectionHealth
            function onStabilityChanged() {
                if (ipInfoPanel.isOpen &&
                    VPNConnectionHealth.stability === VPNConnectionHealth.NoSignal) {
                     ipInfoPanel.isOpen = false;
                 }
            }
        }
        Connections {
            target: VPNController
            function onStateChanged() {
                ipInfoPanel.isOpen = false
            }
        }
    }

    VPNIconButton {
        id: ipInfoToggleButton
        objectName: "ipInfoToggleButton"

        //% "Close"
        property var connectionInfoCloseText: qsTrId("vpn.connectionInfo.close")

        anchors {
            right: parent.right
            rightMargin: VPNTheme.theme.windowMargin / 2
            top: parent.top
            topMargin: VPNTheme.theme.windowMargin / 2
        }
        accessibleName: ipInfoPanel.isOpen
            ? connectionInfoCloseText
            : VPNI18n.ConnectionInfoIpInfoButtonLabel
        buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
        enabled: visible && VPNConnectionHealth.stability !== VPNConnectionHealth.NoSignal
        opacity: visible ? 1 : 0
        visible: connectionInfoToggleButton.visible
            && !connectionInfoScreen.isOpen
            && !connectionInfoScreen.isTransitioning
        z: 1
        onClicked: {
            ipInfoPanel.isOpen = !ipInfoPanel.isOpen;
        }
        Accessible.ignored: !visible

        Image {
            property int iconSize: ipInfoPanel.isOpen
                ? VPNTheme.theme.iconSize
                : VPNTheme.theme.iconSize * 1.5

            anchors.centerIn: ipInfoToggleButton
            source: ipInfoPanel.isOpen
                ? "qrc:/nebula/resources/close-white.svg"
                : "qrc:/nebula/resources/connection-info.svg"
            sourceSize {
                height: iconSize
                width: iconSize
            }
            opacity: parent.enabled ? 1 : .6
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 300
            }
        }
    }

    VPNConnectionInfoScreen {
        id: connectionInfoScreen
        isOpen: box.connectionInfoScreenVisible

        height: boxBackground.height
        radius: VPNTheme.theme.cornerRadius * 2
    }

    Component.onCompleted: VPNNavigator.addView(VPNNavigator.ScreenHome, connectionInfoScreen)

    Connections {
        function onGoBack(item) {
            if (item === connectionInfoScreen && connectionInfoScreen.isOpen) {
                closeConnectionInfo();
            }
        }

        target: VPNNavigator
    }
}
