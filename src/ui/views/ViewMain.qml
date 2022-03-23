/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

import "qrc:/ui"

VPNFlickable {
    id: vpnFlickable

    flickContentHeight: col.height + VPNTheme.theme.windowMargin / 2
    anchors.left: parent.left
    anchors.right: parent.right

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

    Connections {
        target: VPNController
        function onActivationBlockedForCaptivePortal() {
            stackview.push("qrc:/ui/views/ViewCaptivePortalInfo.qml");
        }
    }
    Connections{
        target: VPNCaptivePortal
        function onCaptivePortalPresent() {
            if (VPNController.state != VPNController.StateOff){
                stackview.push("qrc:/ui/views/ViewCaptivePortalInfo.qml");
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: box.connectionInfoVisible
        onClicked: box.closeConnectionInfo()
    }

    GridLayout {
        id: col

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: VPNTheme.theme.windowMargin / 2
        anchors.rightMargin: VPNTheme.theme.windowMargin / 2
        flow: GridLayout.TopToBottom
        rowSpacing: VPNTheme.theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: VPNTheme.theme.windowMargin

        RowLayout {
            id: mobileHeader
            Layout.preferredHeight: VPNTheme.theme.rowHeight
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

        VPNAlerts {
            id: notifications
            updateURL: "qrc:/ui/views/ViewUpdate.qml"
        }

        VPNControllerView {
            id: box
            z: 1
        }

        VPNControllerNav {
            function handleClick() {
                if (disableRowWhen) {
                    return
                }

                stackview.push("ViewServers.qml")
            }

            id: serverInfo
            objectName: "serverListButton"

            //% "Select location"
            //: Select the Location of the VPN server
            titleText: qsTrId("vpn.servers.selectLocation")

            subtitleText: VPNCurrentServer.localizedCityName

            //% "current location - %1"
            //: Accessibility description for current location of the VPN server
            descriptionText: qsTrId("vpn.servers.currentLocation").arg(
                                 VPNCurrentServer.localizedCityName)

            disableRowWhen: (VPNController.state !== VPNController.StateOn
                             && VPNController.state !== VPNController.StateOff)
                            || box.connectionInfoVisible
            Layout.topMargin: 12
            contentChildren: [

                VPNServerLabel {
                    id: selectLocationLabel
                    serversList: [
                        {
                            countryCode: typeof(VPNCurrentServer.entryCountryCode) !== 'undefined' ? VPNCurrentServer.entryCountryCode : "" ,
                            localizedCityName: typeof(VPNCurrentServer.localizedEntryCity) !== 'undefined' ? VPNCurrentServer.localizedEntryCity : "",
                            cityName: typeof(VPNCurrentServer.entryCityName) !== "undefined" ? VPNCurrentServer.entryCityName : ""
                        },
                        {
                         countryCode: VPNCurrentServer.exitCountryCode,
                         localizedCityName: VPNCurrentServer.localizedCityName,
                         cityName: VPNCurrentServer.exitCityName
                        }
                    ]
                }
            ]
        }

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewDevices.qml")
            }

            Layout.topMargin: 6

            objectName: "deviceListButton"
            //% "My devices"
            titleText: qsTrId("vpn.devices.myDevices")
            disableRowWhen: box.connectionInfoVisible
            contentChildren: [
                VPNIcon {
                    source: "qrc:/nebula/resources/devices.svg"
                    Layout.alignment: Qt.AlignLeft | Qt.AlignCenter
                },

                VPNLightLabel {
                    id: serverLocation
                    Accessible.ignored: true
                    Layout.alignment: Qt.AlignLeft
                    elide: Text.ElideRight
                    //% "%1 of %2"
                    //: Example: You have "x of y" devices in your account, where y is the limit of allowed devices.
                    text: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(
                              VPNDeviceModel.activeDevices
                              + (VPN.state !== VPN.StateDeviceLimit ? 0 : 1)).arg(
                              VPNUser.maxDevices)
                }
            ]
        }

        VPNVerticalSpacer {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }

        Connections {
            target: window

            function onShowServersView() {
                stackview.push("ViewServers.qml", StackView.Immediate);
            }
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

        function onContactUsNeeded() {
            if (stackview.currentItem.objectName === "contactUs") return;

            while(stackview.depth > 1) {
                stackview.pop(null, StackView.Immediate);
            }

            stackview.push("qrc:/ui/views/ViewContactUs.qml", { isMainView: false });
        }
    }
}
