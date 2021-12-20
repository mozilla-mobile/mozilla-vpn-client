/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

ColumnLayout {
    id: controller

    property var titleText
    property string subtitleText: ""
    property alias disableRowWhen: btn.rowShouldBeDisabled
    property string descriptionText
    default property alias contentChildren: controllerContent.children
    spacing: 4

    VPNBoldLabel {
        text: titleText
        Layout.leftMargin: VPNTheme.theme.windowMargin
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
        Layout.preferredHeight: VPNTheme.theme.rowHeight

        onClicked: handleClick()

        RowLayout {
            width: parent.width - (VPNTheme.theme.windowMargin * 2)
            anchors.centerIn: parent
            spacing: 0

            RowLayout {
                id: controllerContent
                spacing:8
            }

            VPNChevron {
                id: icon
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
        }

    }
}
