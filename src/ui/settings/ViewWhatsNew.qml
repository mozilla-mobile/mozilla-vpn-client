/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    property string _menuTitle: VPNl18n.WhatsNewReleaseNotesTourPageHeader
    id: root
    objectName: "settingsWhatsNew"

    VPNFlickable {
        id: vpnFlickable
        anchors.top: parent.top
        anchors.topMargin: VPNTheme.theme.menuHeight
        anchors.left: root.left
        anchors.right: root.right
        height: root.height - menu.height
        flickContentHeight: col.height
        interactive: flickContentHeight > height


        ColumnLayout {
            id: col

            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0
            width: parent.width - VPNTheme.theme.windowMargin

            VPNSettingsItem {                
                settingTitle: VPNl18n.WhatsNewReleaseNotesTourSubHeader
                imageLeftSrc: hovered ? "qrc:/ui/resources/magic-purple.svg" : "qrc:/nebula/resources/magic-dark.svg"
                imageRightSrc: hovered ? "qrc:/ui/resources/chevron-purple.svg" : "qrc:/nebula/resources/chevron.svg"
                backgroundColor: VPNTheme.theme.clickableRowPurple
                fontColor: hovered ? VPNTheme.colors.purple70 : VPNTheme.theme.fontColorDark

                Layout.topMargin: VPNTheme.theme.vSpacingSmall

                onClicked: {
                    featureTourPopup.startTour();
                }
            }

            Rectangle {
                id: separatorLine

                color: VPNTheme.colors.grey10
                height: 1
                Layout.bottomMargin:VPNTheme.theme.vSpacingSmall
                Layout.leftMargin: VPNTheme.theme.windowMargin * 2 + 14
                Layout.rightMargin: VPNTheme.theme.windowMargin / 2
                Layout.topMargin: VPNTheme.theme.vSpacingSmall
                Layout.fillWidth: true
            }

            Repeater {
                id: featureRepeater

                model: VPNWhatsNewModel
                delegate: ColumnLayout {
                    Layout.leftMargin: VPNTheme.theme.windowMargin / 2
                    Layout.rightMargin: VPNTheme.theme.windowMargin
                    Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
                    Layout.topMargin: VPNTheme.theme.vSpacingSmall

                    VPNIconAndLabel {
                        id: featureTitle

                        title: feature.displayName
                        icon: feature.iconPath
                        Layout.fillWidth: true
                    }

                    VPNTextBlock {
                        id: featureText

                        text: feature.shortDescription
                        Layout.fillWidth: true
                        Layout.leftMargin: VPNTheme.theme.vSpacing + 14
                    }
                }
            }

        }
    }

}
