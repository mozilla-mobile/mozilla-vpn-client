/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0

MZCard {
    id: root

    property alias imageSrc: heroImage.source
    property string imageBgColor: MZTheme.theme.guideCardImageBgColor
    property alias title: textTitle.text

    Accessible.name: title
    Accessible.ignored: !visible

    ColumnLayout {
        id: layout
        anchors.fill: parent

        spacing: 8

        Rectangle {
            id: imageBg
            Layout.fillWidth: true
            Layout.preferredHeight: 104

            radius: root.radius
            color: root.imageBgColor

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

        MZBoldInterLabel {
            id: textTitle

            Layout.leftMargin: MZTheme.theme.listSpacing
            Layout.rightMargin: MZTheme.theme.listSpacing
            Layout.fillHeight: true
            Layout.fillWidth: true

            elide: Text.ElideRight
            verticalAlignment: Text.AlignTop
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            Accessible.ignored: true
        }
    }
}
