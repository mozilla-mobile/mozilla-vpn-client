/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: root

    property string _menuTitle: VPNl18n.SettingsTipsAndTricks
    readonly property int tabletSize: 600 //Tablets get a different layout (2 columns for tutorials, 3 columns for guides)

    objectName: "settingsTipsAndTricks"

    VPNFlickable {
        id: vpnFlickable

        property int screenWidth: width

        anchors.fill: parent
        anchors.topMargin: VPNTheme.theme.menuHeight

        flickContentHeight: layout.implicitHeight + layout.anchors.topMargin
        interactive: flickContentHeight > height

        ColumnLayout {
            id: layout

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: VPNTheme.theme.vSpacing
            anchors.leftMargin: VPNTheme.theme.windowMargin
            anchors.rightMargin: VPNTheme.theme.windowMargin

            spacing: 0

            Flow {
                Layout.fillWidth: true
                spacing: VPNTheme.theme.vSpacingSmall

                VPNTutorialCard {
                    width: vpnFlickable.screenWidth < root.tabletSize ? parent.width : (parent.width - parent.spacing) / 2
                    height: 144

                    imageSrc: "qrc:/tutorials/01_get_started.svg"
                    imageBgColor: VPNTheme.theme.ink
                    cardTypeText: "Tutorial"
                    title: "Getting started with VPN"
                    description: "Follow this walkthrough to learn how to get started with using your VPN."
                }

                VPNTutorialCard {
                    width: vpnFlickable.screenWidth < root.tabletSize ? parent.width : (parent.width - parent.spacing) / 2
                    height: 144

                    imageSrc: "qrc:/tutorials/01_get_started.svg"
                    imageBgColor: VPNTheme.theme.ink
                    cardTypeText: "Feature tour"
                    title: "Getting started with VPN"
                    description: "Follow this walkthrough to learn how to get started with using your VPN."
                }
            }


            VPNBoldLabel {
                Layout.topMargin: 32
                Layout.fillWidth: true

                text: "Quick tips" //ML TODO: REPLACE WITH L10N
                elide: Text.ElideRight
                lineHeightMode: Text.FixedHeight
                lineHeight: 24
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: 2
                wrapMode: Text.WordWrap

                Accessible.role: Accessible.StaticText
                Accessible.name: text
            }

            VPNTextBlock {
                Layout.topMargin: 4
                Layout.fillWidth: true

                text: "Let us help you get the most out of Mozilla VPN" //ML TODO: REPLACE WITH L10N
            }


            Flow {
                Layout.topMargin: VPNTheme.theme.vSpacingSmall
                Layout.fillWidth: true
                spacing: 16

                Repeater {
                    model: VPNGuide
                    delegate: VPNGuideCard {
                        height: 172
                        width: vpnFlickable.screenWidth < root.tabletSize ? (parent.width - parent.spacing) / 2 : (parent.width - (parent.spacing * 2)) / 3

                        imageSrc: guide.image
                        title: VPNl18n[guide.titleId]
                    }
                }
            }

            VPNTutorialCard {
                Layout.topMargin: 32
                Layout.preferredWidth: vpnFlickable.screenWidth < root.tabletSize ? parent.width : (parent.width - parent.spacing) / 2
                Layout.preferredHeight: 144

                imageSrc: "qrc:/tutorials/01_get_started.svg"
                imageBgColor: VPNTheme.theme.ink
                cardTypeText: "Tutorial"
                title: "Getting started with VPN"
                description: "Follow this walkthrough to learn how to get started with using your VPN."
            }

            //padding for the bottom of the flickable
            Item {
                Layout.preferredHeight: 66
            }
        }
    }
}
