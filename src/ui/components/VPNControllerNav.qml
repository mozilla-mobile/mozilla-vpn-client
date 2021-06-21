/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

ColumnLayout {
    id: controller

    property var titleText
    property var subtitleText
    property var descriptionText
    property var imgSource
    property var imgSize: 20
    property var imgIsVector: false
    property var disableRowWhen
    spacing: 4

    VPNBoldLabel {
        text: titleText
        Layout.leftMargin: Theme.windowMargin
        opacity: disableRowWhen ?  .7 : 1

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    VPNClickableRow {
        id: btn
        accessibleName: titleText + ": " + descriptionText
        Accessible.ignored: rowShouldBeDisabled
        activeFocusOnTab: true
        anchors.left: undefined
        anchors.right: undefined
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: Theme.rowHeight

        onClicked: handleClick()
        rowShouldBeDisabled: disableRowWhen

        RowLayout {
            width: parent.width - (Theme.windowMargin * 2)
            anchors.centerIn: parent
            spacing: 0

            RowLayout {
                spacing:8

                Rectangle {
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.alignment: Qt.AlignLeft | Qt.AlignCenter
                    color: "transparent"

                    Image {
                        id: flag
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        width: imgIsVector? undefined : imgSize
                        sourceSize.width: imgIsVector ? imgSize : undefined
                        fillMode: Image.PreserveAspectFit
                        source: imgSource
                    }
                }

                VPNLightLabel {
                    id: serverLocation
                    text: subtitleText
                    Accessible.ignored: true
                    Layout.alignment: Qt.AlignLeft
                    elide: Text.ElideRight
                }
            }

            VPNChevron {
                id: icon
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
        }
    }
}
