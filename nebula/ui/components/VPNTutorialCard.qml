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
    property string cardTypeText: VPNl18n.TipsAndTricksTutorialLabel //defaults card type to "tutorial"
    property alias title: titleText.text
    property alias description: descriptionText.text

    Accessible.name: root.cardTypeText + "," + title + "," + description

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
            Layout.topMargin: 16
            Layout.rightMargin: 16
            Layout.bottomMargin: 8

            spacing: 0

            VPNBoldInterLabel {
                id: cardTypeText

                Layout.fillWidth: true

                text: root.cardTypeText
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: 1

                Accessible.ignored: true
            }


            VPNBoldLabel {
                id: titleText

                Layout.topMargin: VPNTheme.theme.listSpacing
                Layout.fillWidth: true

                elide: Text.ElideRight
                lineHeightMode: Text.FixedHeight
                lineHeight: 24
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: 2
                wrapMode: Text.WordWrap

                Accessible.role: Accessible.StaticText
                Accessible.name: text
                Accessible.ignored: true
            }

            Text {
                id: descriptionText

                Layout.fillWidth: true
                Layout.fillHeight: true

                topPadding: 4
                elide: Text.ElideRight
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: VPNTheme.theme.fontSizeSmallest
                font.family: VPNTheme.theme.fontInterFamily
                lineHeightMode: Text.FixedHeight
                lineHeight: VPNTheme.theme.controllerInterLineHeight
                wrapMode: Text.WordWrap
                color: VPNTheme.theme.fontColor

                Accessible.role: Accessible.StaticText
                Accessible.name: text
                Accessible.ignored: true
            }
        }
    }
}
