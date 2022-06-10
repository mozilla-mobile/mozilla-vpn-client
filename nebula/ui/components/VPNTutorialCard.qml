/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

VPNCard {
    id: root

    property alias imageSrc: heroImage.source
    property string imageBgColor: VPNTheme.theme.tutorialCardImageBgColor
    property alias title: titleText.text
    property alias description: descriptionText.text

    Accessible.name: title + "," + description

    RowLayout {
        anchors.fill: parent

        spacing: VPNTheme.theme.windowMargin

        Rectangle {
            id: imageBg
            Layout.fillHeight: true
            Layout.preferredWidth: 112

            radius: root.radius
            color: root.imageBgColor

            //Used to get flat edges at the bottom of hero image background
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                width: parent.radius
                color: parent.color
            }

            Image {
                id: heroImage

                anchors.centerIn: parent
                sourceSize.height: 80
                sourceSize.width: 80
            }
        }

        ColumnLayout {
            Layout.topMargin: VPNTheme.theme.vSpacing * 0.5
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.bottomMargin: VPNTheme.theme.vSpacing * 0.5
            Layout.fillHeight: true
            Layout.fillWidth: true

            spacing: 0

            Item {
                Layout.fillHeight: true
            }

            VPNBoldLabel {
                id: titleText

                Layout.fillWidth: true

                elide: Text.ElideRight
                lineHeightMode: Text.FixedHeight
                lineHeight: VPNTheme.theme.vSpacing
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                maximumLineCount: 2
                wrapMode: Text.WordWrap

                Accessible.role: Accessible.StaticText
                Accessible.name: text
                Accessible.ignored: true
            }

            Text {
                id: descriptionText

                Layout.fillWidth: true
                Layout.maximumHeight: {
                    //Max height should be all available space rounded down to the nearlest line (so there is no excess height like half of a line)
                    const availableHeight = root.height - parent.Layout.topMargin - parent.Layout.bottomMargin - titleText.height
                    const excessHeight = availableHeight % lineHeight
                    return root.height - parent.Layout.topMargin - parent.Layout.bottomMargin - titleText.height - excessHeight
                }

                elide: Text.ElideRight
                font.pixelSize: VPNTheme.theme.fontSizeSmallest
                font.family: VPNTheme.theme.fontInterFamily
                lineHeightMode: Text.FixedHeight
                lineHeight: VPNTheme.theme.controllerInterLineHeight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
                color: VPNTheme.theme.fontColor

                Accessible.role: Accessible.StaticText
                Accessible.name: text
                Accessible.ignored: true
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
