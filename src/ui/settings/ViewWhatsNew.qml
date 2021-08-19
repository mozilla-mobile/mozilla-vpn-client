/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

Item {
    id: root

    property variant featureData: [
        {
            title: "Multi-hop VPN",
            textShort: "Multi-hop VPN uses multiple VPN servers instead of one for extra security and privacy.",
            imageSrc: "../resources/shield-off.svg",
            icon: "../resources/location-dark.svg"
        },
        {
            title: "In-app Support Form",
            textShort: "The in-app support form will streamline the the process of getting your issues resolved.",
            imageSrc: "../resources/globe.svg",
            icon: "../resources/settings/getHelp.svg",
        },
        {
            title: "Custom DNS",
            textShort: "Custom DNS settings allows for faster connection using local servers.",
            imageSrc: "../resources/shield-on.svg",
            icon: "../resources/settings/networkSettings.svg",
        },
    ]

    VPNMenu {
        id: menu
        objectName: "settingsWhatsNew"

        title: "What’s new"
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        anchors.top: menu.bottom
        anchors.left: root.left
        anchors.right: root.right
        height: root.height - menu.height
        flickContentHeight: col.height
        interactive: flickContentHeight > height

        ColumnLayout {
            id: col

            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0
            width: parent.width - Theme.windowMargin

            VPNSettingsItem {
                settingTitle: "Take the tour"
                imageLeftSrc: "../resources/magic-dark.svg"
                imageRightSrc: "../resources/chevron.svg"
                Layout.topMargin: 16
                onClicked: {
                    featureTourPopup.openTour();
                }
            }

            Rectangle {
                id: separatorLine

                color: Color.grey10
                height: 1
                Layout.bottomMargin: 16
                Layout.topMargin: 16
                Layout.fillWidth: true
            }
            
            Repeater {
                model: VPNFeatureList
                
                delegate: Button {
                    text: index
                }
            }

            Component {
                id: featureItem

                ColumnLayout {
                    Layout.leftMargin: Theme.windowMargin / 2
                    Layout.rightMargin: Theme.windowMargin
                    Layout.bottomMargin: 16
                    Layout.topMargin: 16

                    VPNIconAndLabel {
                        id: featureTitle

                        icon: modelData.icon
                        title: modelData.title
                    }

                    VPNTextBlock {
                        id: featureText
                        text: modelData.textShort
                        Layout.fillWidth: true
                        Layout.leftMargin: 38
                    }
                }
            }

            Repeater {
                id: featureRepeater

                model: featureData
                delegate: featureItem
            }
        }
    }
}
