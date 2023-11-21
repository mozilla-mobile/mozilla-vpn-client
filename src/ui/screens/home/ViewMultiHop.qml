/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

StackView {
    id: multiHopStackView
    objectName: "multiHopStackView"

    Component.onCompleted: function() {
        MZNavigator.addStackView(VPN.ScreenHome, multiHopStackView)
    }

    Connections {
        target: MZNavigator
        function onGoBack(item) {
            if (item === multiHopStackView) {
                multiHopStackView.pop();
            }
        }
    }

    initialItem: MZFlickable {

        id: vpnFlickable

        flickContentHeight: col.implicitHeight
        contentHeight: flickContentHeight

        ColumnLayout {
            id: col
            anchors.top: parent.top
            anchors.topMargin: MZTheme.theme.windowMargin
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: MZTheme.theme.windowMargin / 2
            anchors.rightMargin: MZTheme.theme.windowMargin / 2
            spacing: MZTheme.theme.vSpacing

            MZCollapsibleCard {
                title: MZI18n.MultiHopFeatureMultiHopCardHeader

                iconSrc: "qrc:/ui/resources/tip.svg"
                contentItem: MZTextBlock {
                    text: MZI18n.MultiHopFeatureMultiHopCardBody
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width - MZTheme.theme.windowMargin
            }

            RecentConnections {
                Layout.fillWidth: true
                showMultiHopRecentConnections: true
            }

            ControllerNav {
                function handleClick() {
                    multiHopStackView.push(
                        "qrc:/ui/screens/home/servers/ServerList.qml",
                        {
                            currentServer: entryLabel.serversList[0],
                            showRecentConnections: false
                        }
                    );
                    menu.title =  titleText;
                }

                titleText: MZI18n.MultiHopFeatureMultiHopEntryLocationHeader
                // If the current VPN configuration is single hop, VPNCurrentServer.localizedEntryCityName will be empty, so use multiHopEntryServer
                descriptionText: segmentedNav.multiHopEntryServer[2]
                contentChildren: [
                    ServerLabel {
                        id: entryLabel
                        objectName: "buttonSelectEntry"

                        serversList: [
                            // Multi-hop TODO : What to do when there is no entry server.
                            {
                                countryCode: typeof(segmentedNav.multiHopEntryServer[0]) !== "undefined" ? segmentedNav.multiHopEntryServer[0] : "",
                                cityName: typeof(segmentedNav.multiHopEntryServer[1]) !== "undefined" ? segmentedNav.multiHopEntryServer[1] : "",
                                localizedCityName: typeof(segmentedNav.multiHopEntryServer[2]) !== 'undefined' ? segmentedNav.multiHopEntryServer[2] : "",
                                whichHop: "multiHopEntryServer"
                            }
                        ]
                    }
                ]
            }

            ColumnLayout {
                spacing: 8
                ControllerNav {
                    function handleClick() {
                        multiHopStackView.push("qrc:/ui/screens/home/servers/ServerList.qml",
                           {
                                currentServer:  exitLabel.serversList[0],
                                showRecentConnections: false
                           });

                        menu.title = titleText;
                    }

                    btnObjectName: "buttonSelectExit-btn"
                    titleText: MZI18n.MultiHopFeatureMultiHopExitLocationHeader
                    descriptionText: segmentedNav.multiHopExitServer[2]
                    contentChildren: [

                        ServerLabel {
                            id: exitLabel
                            objectName: "buttonSelectExit"

                            serversList: [

                                {
                                    countryCode: segmentedNav.multiHopExitServer[0],
                                    cityName: segmentedNav.multiHopExitServer[1],
                                    localizedCityName : segmentedNav.multiHopExitServer[2],
                                    whichHop: "multiHopExitServer"
                                }
                            ]
                        }
                    ]
                }
                RowLayout {
                    id: turnVPNOffAlert
                    spacing: 4
                    Layout.fillWidth: true
                    Layout.preferredHeight: MZTheme.theme.rowHeight

                    Rectangle {
                        color: MZTheme.theme.transparent
                        Layout.preferredHeight: MZTheme.theme.vSpacing
                        Layout.preferredWidth: MZTheme.theme.vSpacing
                        Layout.leftMargin: 12
                        Layout.alignment: Qt.AlignTop

                        MZIcon {
                            id: warningIcon
                            source: "qrc:/ui/resources/connection-info-dark.svg"
                            sourceSize.height: 18
                            sourceSize.width: 18
                            anchors.centerIn: parent
                        }
                    }

                    MZTextBlock {
                        id: message
                        text: MZI18n.MultiHopFeatureMultiHopInfoText
                        color: MZTheme.colors.grey40
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: 4
                    }
                }
            }

        }
    }
}
