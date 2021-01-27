/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14

import "../themes/themes.js" as Theme
ColumnLayout {
    id: panel

    property alias logo: logo.source
    property alias logoTitle: logoTitle.text
    property alias logoSubtitle: logoSubtitle.text
    property var logoSize: 76
    property var maskImage: false
    property var imageIsVector: true

    width:  Math.min(parent.width * .8, Theme.maxHorizontalContentWidth - Theme.windowMargin * 4)
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: 0

    ColumnLayout {
        id: contentWrapper

        Layout.alignment: Qt.AlignCenter
        spacing: 0

        Rectangle {
            id: logoWrapper

            color: "transparent"
            Layout.preferredHeight: Math.max(logoSize, 76)
            Layout.fillWidth: true

            Image {
                id: logo

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: logoWrapper.bottom
                verticalAlignment: Image.AlignBottom
                anchors.bottomMargin: 0
                sourceSize.height: panel.imageIsVector ? logoSize : undefined
                sourceSize.width: panel.imageIsVector ? logoSize : undefined
                fillMode: Image.PreserveAspectFit
                layer.enabled: true
                Component.onCompleted: {
                    if (!panel.imageIsVector) {
                        logo.height = logoSize;
                        logo.width = logoSize;
                        logo.smooth = true;
                    }
                }

                Rectangle {
                    id: mask

                    anchors.fill: parent
                    radius: logoSize / 2
                    visible: false
                }

                layer.effect: OpacityMask {
                    maskSource: panel.maskImage ? mask : undefined
                }

            }

        }

        VPNHeadline {
            id: logoTitle

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 24
            Layout.fillWidth: true
        }

        VPNSubtitle {
            id: logoSubtitle

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 12
            Layout.fillWidth: true
        }

    }

}
