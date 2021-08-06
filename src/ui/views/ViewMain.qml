/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

VPNFlickable {
    id: vpnFlickable

    flickContentHeight: col.height + Theme.windowMargin / 2
    states: [
        State {
            when: window.fullscreenRequired()

            PropertyChanges {
                target: mainView
            }
            PropertyChanges {
                target: mobileHeader
                visible: true
            }
        },
        State {
            when: !window.fullscreenRequired()
            PropertyChanges {
                target: mainView
            }
            PropertyChanges {
                target: mobileHeader
                visible: false
            }
        }
    ]

    MouseArea {
        anchors.fill: parent
        enabled: box.connectionInfoVisible
        onClicked: box.closeConnectionInfo()
    }

    GridLayout {
        id: col
        width: parent.width - Theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter
        flow: GridLayout.TopToBottom
        rowSpacing: Theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin

        RowLayout {
            id: mobileHeader
            Layout.preferredHeight: Theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            spacing: 6

            VPNIcon {
                source: VPNStatusIcon.iconUrl
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
                Layout.alignment: Qt.AlignVCenter
            }

            VPNBoldLabel {
                //% "Mozilla VPN"
                text: qsTrId("MozillaVPN")
                color: "#000000"
                Layout.alignment: Qt.AlignVCenter
            }
        }

        VPNCollapsibleCard {
            title: VPNl18n.tr(VPNl18n.MultiHopHintCardTitle)
            iconSrc: "../resources/tip.svg"
            contentItem: VPNTextBlock {
                color: Color.grey40
                text: VPNl18n.tr(VPNl18n.MultiHopHintCardTextOne) + " <b>" + VPNl18n.tr(VPNl18n.MultiHopHintCardTextTwo) + "</b>"
                textFormat: Text.StyledText
                Layout.fillWidth: true
            }
            z: 1
        }

        VPNAlerts {
            id: notifications
        }

        VPNControllerView {
            id: box
        }

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewServers.qml")
            }

            Layout.topMargin: 12

            id: serverInfo
            objectName: "serverListButton"

            //% "Select location"
            //: Select the Location of the VPN server
            titleText: qsTrId("vpn.servers.selectLocation")
            //% "current location - %1"
            //: Accessibility description for current location of the VPN server
            descriptionText: qsTrId("vpn.servers.currentLocation").arg(VPNCurrentServer.localizedCityName)

            subtitleText: VPNCurrentServer.localizedCityName
            imgSource:  "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase() + ".png"
            disableRowWhen: (VPNController.state !== VPNController.StateOn && VPNController.state !== VPNController.StateOff) || box.connectionInfoVisible
        }

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewDevices.qml")
            }

            Layout.topMargin: 6

            objectName: "deviceListButton"
            //% "%1 of %2"
            //: Example: You have "x of y" devices in your account, where y is the limit of allowed devices.
            subtitleText: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(VPNDeviceModel.activeDevices + (VPN.state !== VPN.StateDeviceLimit ? 0 : 1)).arg(VPNUser.maxDevices)
            imgSource: "../resources/devices.svg"
            imgIsVector: true
            imgSize: 24
            //% "My devices"
            titleText: qsTrId("vpn.devices.myDevices")
            disableRowWhen: box.connectionInfoVisible
        }

        VPNVerticalSpacer {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }
    }
}
