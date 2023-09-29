/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

MZFlickable {
    id: vpnFlickable

    objectName: "viewMainFlickable"

    flickContentHeight: col.height + col.anchors.topMargin
    anchors.left: parent.left
    anchors.right: parent.right

    Component.onCompleted: {
        Glean.sample.appStep.record({
            action: "impression",
            screen: "main",
        });
    }

    states: [
        State {
            when: window.fullscreenRequired()
            PropertyChanges {
                target: mobileHeader
                visible: true
            }
        },
        State {
            when: !window.fullscreenRequired()
            PropertyChanges {
                target: mobileHeader
                visible: false
            }
        }
    ]

    GridLayout {
        id: col

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: MZTheme.theme.windowMargin / 2
        anchors.rightMargin: MZTheme.theme.windowMargin / 2
        flow: GridLayout.TopToBottom
        rowSpacing: MZTheme.theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: MZTheme.theme.windowMargin

        RowLayout {
            id: mobileHeader
            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            spacing: 6

            MZIcon {
                source: "qrc:/ui/resources/logo.svg"
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
                Layout.alignment: Qt.AlignVCenter
            }

            MZBoldLabel {
                //% "Mozilla VPN"
                text: qsTrId("MozillaVPN")
                color: "#000000"
                Layout.alignment: Qt.AlignVCenter
            }
        }

        ControllerView {
            id: box
            z: 1
        }

        ControllerNav {
            function handleClick() {
                if (disableRowWhen) {
                    return
                }

                stackview.push("ViewServers.qml")
            }

            id: serverInfo

            objectName: "serverListButton"
            btnObjectName: "serverListButton-btn"

            //% "Select location"
            //: Select the Location of the VPN server
            titleText: qsTrId("vpn.servers.selectLocation")

            //% "current location - %1"
            //: Accessibility description for current location of the VPN server
            descriptionText: qsTrId("vpn.servers.currentLocation").arg(
                                 VPNCurrentServer.localizedExitCityName)

            disableRowWhen: (VPNController.state !== VPNController.StateOn
                             && VPNController.state !== VPNController.StateOff)
                            || box.connectionInfoScreenVisible
            Layout.topMargin: 12
            contentChildren: [

                ServerLabel {
                    id: selectLocationLabel
                    objectName: "serverListButton-label"

                    serversList: [
                        {
                            countryCode: typeof(VPNCurrentServer.entryCountryCode) !== 'undefined' ? VPNCurrentServer.entryCountryCode : "" ,
                            localizedCityName: typeof(VPNCurrentServer.localizedEntryCityName) !== 'undefined' ? VPNCurrentServer.localizedEntryCityName : "",
                            cityName: typeof(VPNCurrentServer.entryCityName) !== "undefined" ? VPNCurrentServer.entryCityName : ""
                        },
                        {
                         countryCode: VPNCurrentServer.exitCountryCode,
                         localizedCityName: VPNCurrentServer.localizedExitCityName,
                         cityName: VPNCurrentServer.exitCityName
                        }
                    ]
                }
            ]
        }
    }
}
