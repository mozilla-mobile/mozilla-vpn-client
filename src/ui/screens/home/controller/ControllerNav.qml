/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    id: controller

    property var titleText
    property alias disableRowWhen: btn.rowShouldBeDisabled
    property alias btnObjectName: btn.objectName
    property string descriptionText
    default property alias contentChildren: controllerContent.children
    spacing: 4

    MZBoldLabel {
        text: titleText
        Layout.leftMargin: MZTheme.theme.windowMargin
        opacity: disableRowWhen ?  .7 : 1

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    MZClickableRow {
        id: btn

        accessibleName: titleText + ": " + descriptionText
        Accessible.ignored: rowShouldBeDisabled || !visible
        activeFocusOnTab: true
        anchors.left: undefined
        anchors.right: undefined
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: MZTheme.theme.rowHeight

        onClicked: handleClick()

        RowLayout {
            width: parent.width - (MZTheme.theme.windowMargin * 2)
            anchors.centerIn: parent
            spacing: 0

            RowLayout {
                id: controllerContent
                spacing:8
            }

            MZChevron {
                id: icon
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            }
        }

    }
}
