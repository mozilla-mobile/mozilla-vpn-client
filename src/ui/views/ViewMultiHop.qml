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
        flickContentHeight: col.implicitHeight - 56 * 2

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
                title: VPNl18n.tr(VPNl18n.MultiHopFeatureCardHeader)

                iconSrc: "../resources/tip.svg"
                contentItem: VPNTextBlock {
                    text: VPNl18n.tr(VPNl18n.MultiHopFeatureCardBody)
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width - Theme.windowMargin
            }


            VPNRecentConnections {
                Layout.fillWidth: true
                recentConnectionsList: recentConnectionsListModel

            }


            VPNControllerNav {
                function handleClick() {
                    multiHopStackView.push("../components/VPNServerList.qml");
                    menu.title =  titleText;
                }

                objectName: "buttonSelectEntry"
                titleText: VPNl18n.tr(VPNl18n.MultiHopFeatureEntryLocationHeader)
                descriptionText: titleText
                contentChildren: [
                    VPNServerLabel {

                        // MULTIHOP TODO - Replace with real thing
                        serversList: currentServers.get(0)
                    }
                ]
            }

            VPNControllerNav {
                function handleClick() {
                    multiHopStackView.push("../components/VPNServerList.qml");
                    menu.title = titleText;
                }

                objectName: "buttonSelectExit"
                titleText: VPNl18n.tr(VPNl18n.MultiHopFeatureExitLocationHeader)
                descriptionText: titleText
                contentChildren: [

                    VPNServerLabel {
                        // MULTIHOP TODO - Replace with real thing
                        serversList: currentServers.get(1)
                    }
                ]
            }
        }
    }
}
