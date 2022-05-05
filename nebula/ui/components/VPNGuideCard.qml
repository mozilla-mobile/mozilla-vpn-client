/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

VPNCard {
    id: root

    property alias imageSrc: heroImage.source
    property alias title: textTitle.text

    ColumnLayout {
        id: layout
        anchors.fill: parent

        spacing: 8

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 104

            radius: root.radius
            color: VPNTheme.colors.pink90

            //Used to get flat edges at the bottom of hero image background
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                height: parent.radius
                color: parent.color
            }

            Image {
                id: heroImage

                anchors.fill: parent
                anchors.topMargin: 14
                anchors.leftMargin: 5.75
                anchors.rightMargin: 5.75
                anchors.bottomMargin: 14
                fillMode: Image.PreserveAspectFit
            }
        }

        Text {
            id: textTitle
            Layout.leftMargin: VPNTheme.theme.listSpacing
            Layout.rightMargin: VPNTheme.theme.listSpacing
            Layout.maximumHeight: lineHeight * 3
            Layout.maximumWidth: parent.width - Layout.leftMargin - Layout.rightMargin

            elide: Text.ElideRight
            font.pixelSize: VPNTheme.theme.fontSizeSmallest
            font.family: VPNTheme.theme.fontInterSemiBoldFamily
            lineHeightMode: Text.FixedHeight
            lineHeight: VPNTheme.theme.controllerInterLineHeight
            verticalAlignment: Text.AlignTop
            wrapMode: Text.WordWrap
            color: VPNTheme.theme.fontColorDark

            Accessible.role: Accessible.StaticText
            Accessible.name: text
        }

        VPNVerticalSpacer {
            Layout.fillHeight: true
        }

    }
}
