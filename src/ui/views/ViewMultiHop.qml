/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/colors.js" as Color
import "../themes/themes.js" as Theme
import "../components"

StackView {
    id: multiHopStackView

    initialItem: VPNFlickable {
        id: vpnFlickable
        flickContentHeight: col.implicitHeight + col.y + Theme.windowMargin
        windowHeightExceedsContentHeight: parent.height > flickContentHeight
        contentHeight: flickContentHeight

        ColumnLayout {
            id: col
            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Theme.windowMargin / 2
            anchors.rightMargin: Theme.windowMargin / 2
            spacing: Theme.vSpacing

            VPNCollapsibleCard {
                title: VPNl18n.tr(VPNl18n.MultiHopFeatureMultiHopCardHeader)

                iconSrc: "../resources/tip.svg"
                contentItem: VPNTextBlock {
                    text: VPNl18n.tr(VPNl18n.MultiHopFeatureMultiHopCardBody)
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width - Theme.windowMargin
            }

            VPNRecentConnections {
                Layout.fillWidth: true
                showMultiHopRecentConnections: true
            }


            VPNControllerNav {
                function handleClick() {
                    multiHopStackView.push(
                        "../components/VPNServerList.qml",
                        {
                                    currentServer: entryLabel.serversList[0],
                                    showRecentConnections: false
                                }
                    );
                    menu.title =  titleText;
                }

                objectName: "buttonSelectEntry"
                titleText: VPNl18n.tr(VPNl18n.MultiHopFeatureMultiHopEntryLocationHeader)
                descriptionText: titleText
                contentChildren: [
                    VPNServerLabel {
                        id: entryLabel

                        // MULTIHOP TODO - Replace with real thing
                        serversList: [
                            {
                                countryCode: typeof(VPNCurrentServer.entryCountryCode) !== 'undefined' ? VPNCurrentServer.entryCountryCode : "" ,
                                localizedCityName: typeof(VPNCurrentServer.localizedEntryCity) !== 'undefined' ? VPNCurrentServer.localizedEntryCity : "",
                                cityName: typeof(VPNCurrentServer.entryCityName) !== "undefined" ? VPNCurrentServer.entryCityName : "",
                                selectWhichHop: "multiHopEntry"
                            }
                        ]
                    }
                ]
            }

            VPNControllerNav {
                function handleClick() {
                    multiHopStackView.push("../components/VPNServerList.qml",
                       {
                           currentServer:  exitLabel.serversList[0],
                                               showRecentConnections: false
                       });
                    menu.title = titleText;
                }

                objectName: "buttonSelectExit"
                titleText: VPNl18n.tr(VPNl18n.MultiHopFeatureMultiHopExitLocationHeader)
                descriptionText: titleText
                contentChildren: [

                    VPNServerLabel {
                        id: exitLabel
                        serversList: [
                            {
                                countryCode: VPNCurrentServer.exitCountryCode,
                                localizedCityName : VPNCurrentServer.localizedCityName,
                                cityName: VPNCurrentServer.exitCityName,
                                selectWhichHop: "multiHopExit"
                            }
                        ]
                    }
                ]
            }
        }
    }
}
