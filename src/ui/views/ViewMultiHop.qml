/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
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

                // MULTIHOP TODO - Use real strings
                title: "Need string"

                iconSrc: "../resources/tip.svg"
                contentItem: VPNTextBlock {
                    text: "This is the collapsible card body content. It any item and is not only displaying text."
                    textFormat: Text.StyledText
                    Layout.fillWidth: true
                }
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: parent.width - Theme.windowMargin
            }


            VPNRecentConnections {
                Layout.fillWidth: true
            }


            VPNControllerNav {
                function handleClick() {
                    multiHopStackView.push("../components/VPNServerList.qml");
                    menu.title =  titleText;
                }

                // MULTIHOP TODO - Replace with real string
                titleText: "String - Select entry location"

                descriptionText: titleText
                contentChildren: [
                    VPNServerLabel {

                        // MULTIHOP TODO - Replace with real server list
                        servers: [
                            {
                                countryCode: "BE",
                                localizedCityName: "Berlin"
                            }
                        ]
                    }
                ]
            }

            VPNControllerNav {
                function handleClick() {
                    multiHopStackView.push("../components/VPNServerList.qml");
                    menu.title = titleText;
                }

                // MULTIHOP TODO - Replace with real string
                titleText: "String - Select exit location" // TODO

                descriptionText: titleText
                contentChildren: [

                    VPNServerLabel {

                        // MULTIHOP TODO - Replace with real server list
                        servers: [
                            {
                                countryCode: "US",
                                localizedCityName: "Dallas"
                            }
                        ]
                    }
                ]
            }
        }
    }
}
