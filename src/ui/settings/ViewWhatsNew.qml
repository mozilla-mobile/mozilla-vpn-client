/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    id: root

    property variant featureData: [
        {
            title: "Multi-hop VPN",
            text: "Multi-hop VPN will route your traffic thourgh a second server for added protection. You can find this feature on the “Select location” screen.",
            imageSrc: "../resources/shield-off.svg",
            icon: "../resources/location-dark.svg"
        },
        {
            title: "In-app Support Form",
            text: "The In-app support form will allow you to contact support from within the VPN app. You can find this feature in the “Get help” section.",
            imageSrc: "../resources/globe.svg",
            icon: "../resources/settings/getHelp.svg",
        },
        {
            title: "Custom DNS",
            text: "Custom DNS servers allow for faster speed using local networks, features like ad-blocking and anti-tracking. You can find this feature in “Network settings” section.",
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
            width: parent.width - Theme.windowMargin

            VPNSettingsItem {
                settingTitle: "Take the tour"
                imageLeftSrc: "../resources/magic-dark.svg"
                imageRightSrc: "../resources/chevron.svg"
                onClicked: featureTourPopup.open()
            }

            Component {
                id: featureItem

                ColumnLayout {
                    VPNIconAndLabel {
                        id: featureTitle

                        icon: modelData.icon
                        title: modelData.title
                    }

                    VPNTextBlock {
                        id: featureText
                        text: modelData.text
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
