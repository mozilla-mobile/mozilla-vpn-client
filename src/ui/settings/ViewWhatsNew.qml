/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.12

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

Item {
    id: root

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
                imageLeftSrc: hovered ? "../resources/magic-purple.svg" : "../resources/magic-dark.svg"
                imageRightSrc: hovered ? "../resources/chevron-purple.svg" : "../resources/chevron.svg"
                backgroundColor: Theme.clickableRowPurple
                fontColor: hovered ? Color.purple70 : Theme.fontColorDark

                Layout.topMargin: Theme.vSpacingSmall

                onClicked: {
                    featureTourPopup.startTour();
                }
            }

            Rectangle {
                id: separatorLine

                color: Color.grey10
                height: 1
                Layout.bottomMargin:Theme.vSpacingSmall
                Layout.leftMargin: Theme.windowMargin * 2 + 14
                Layout.rightMargin: Theme.windowMargin / 2
                Layout.topMargin: Theme.vSpacingSmall
                Layout.fillWidth: true
            }

            Component {
                id: featureItem

                ColumnLayout {
                    Layout.leftMargin: Theme.windowMargin / 2
                    Layout.rightMargin: Theme.windowMargin
                    Layout.bottomMargin: Theme.vSpacingSmall
                    Layout.topMargin: Theme.vSpacingSmall

                    VPNIconAndLabel {
                        id: featureTitle

                        icon: modelData.icon
                        title: modelData.title
                    }

                    VPNTextBlock {
                        id: featureText
                        text: modelData.textShort
                        Layout.fillWidth: true
                        Layout.leftMargin: Theme.vSpacing + 14
                    }
                }
            }

            Repeater {
                id: featureRepeater

                model: featureTourPopup.testData
                delegate: featureItem
            }
        }
    }
}
