/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

ColumnLayout {
    id: controller

    property var titleText
    property var subtitleText
    property var descriptionText
    property var imgSource
    property var imgSize: Theme.windowMargin
    property var imgIsVector: false
    property var disableRowWhen
    spacing: 4
    width: parent.width - Theme.windowMargin
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.topMargin: 24
    state: ""
    states: [
        State {
            name: "deviceLimit"
            PropertyChanges {
                target: controller
                subtitleText: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(VPNDeviceModel.activeDevices + 1).arg(VPNUser.maxDevices)
            }

            PropertyChanges {
                target: icon
                source: "../resources/warning.svg"
                sourceSize.height: 14
                sourceSize.width: 14
                Layout.rightMargin: 6
                Layout.leftMargin: 6
            }
        }
    ]

    VPNBoldLabel {
        text: titleText
        Layout.leftMargin: Theme.windowMargin
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

                Image {
                    id: flag

                    Layout.preferredWidth: imgIsVector? undefined : imgSize
                    Layout.preferredHeight: imgIsVector? undefined : imgSize
                    sourceSize.height: imgIsVector ? imgSize : undefined
                    sourceSize.width: imgIsVector ? imgSize : undefined
                    Layout.alignment: Qt.AlignLeft
                    fillMode: Image.PreserveAspectFit
                    source: imgSource

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
