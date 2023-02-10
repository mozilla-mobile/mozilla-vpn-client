/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

StackView {
    id: multiHopStackView

    Component.onCompleted: function() {
        VPNNavigator.addStackView(VPNNavigator.ScreenHome, multiHopStackView)
    }

    Connections {
        target: VPNNavigator
        function onGoBack(item) {
            if (item === multiHopStackView) {
                multiHopStackView.pop();
            }
        }
    }

    initialItem: VPNFlickable {

        id: vpnFlickable

        flickContentHeight: col.implicitHeight
        contentHeight: flickContentHeight

        ColumnLayout {
            id: col
            anchors.top: parent.top
            anchors.topMargin: VPNTheme.theme.windowMargin
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: VPNTheme.theme.windowMargin / 2
            anchors.rightMargin: VPNTheme.theme.windowMargin / 2
            spacing: VPNTheme.theme.vSpacing

            VPNCollapsibleCard {
                title: VPNI18n.MultiHopFeatureMultiHopCardHeader

                iconSrc: "qrc:/ui/resources/tip.svg"
                contentItem: VPNTextBlock {
                    text: VPNI18n.MultiHopFeatureMultiHopCardBody
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin
            }

            VPNRecentConnections {
                Layout.fillWidth: true
                showMultiHopRecentConnections: true
            }

            VPNControllerNav {
                function handleClick() {
                    multiHopStackView.push(
                        "qrc:/nebula/components/VPNServerList.qml",
                        {
                            currentServer: entryLabel.serversList[0],
                            showRecentConnections: false
                        }
                    );
                    menu.title =  titleText;
                }

                objectName: "buttonSelectEntry"
                titleText: VPNI18n.MultiHopFeatureMultiHopEntryLocationHeader
                descriptionText: titleText
                contentChildren: [
                    VPNServerLabel {
                        id: entryLabel

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
                VPNControllerNav {
                    function handleClick() {
                        multiHopStackView.push("qrc:/nebula/components/VPNServerList.qml",
                           {
                                currentServer:  exitLabel.serversList[0],
                                showRecentConnections: false
                           });

                        menu.title = titleText;
                    }

                    objectName: "buttonSelectExit"
                    btnObjectName: "buttonSelectExit-btn"
                    titleText: VPNI18n.MultiHopFeatureMultiHopExitLocationHeader
                    descriptionText: titleText
                    contentChildren: [

                        VPNServerLabel {
                            id: exitLabel
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
                    Layout.preferredHeight: VPNTheme.theme.rowHeight

                    Rectangle {
                        color: VPNTheme.theme.transparent
                        Layout.preferredHeight: VPNTheme.theme.vSpacing
                        Layout.preferredWidth: VPNTheme.theme.vSpacing
                        Layout.leftMargin: 12
                        Layout.alignment: Qt.AlignTop

                        VPNIcon {
                            id: warningIcon
                            source: "qrc:/ui/resources/connection-info-dark.svg"
                            sourceSize.height: 18
                            sourceSize.width: 18
                            anchors.centerIn: parent
                        }
                    }

                    VPNTextBlock {
                        id: message
                        text: VPNI18n.MultiHopFeatureMultiHopInfoText
                        color: VPNTheme.colors.grey40
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: 4
                    }
                }
            }

        }
    }
}
