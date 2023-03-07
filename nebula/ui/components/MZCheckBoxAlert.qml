/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0

RowLayout {
    property var leftMargin:  MZTheme.theme.menuHeight
    property var alertColor: MZTheme.theme.checkBoxWarning
    property var errorMessage

    id: turnVPNOffAlert

    visible: (VPNController.state !== VPNController.StateOff)
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.rightMargin: MZTheme.theme.windowMargin
    spacing: 0
    anchors.leftMargin: MZTheme.theme.windowMargin

    Rectangle {
        color: MZTheme.theme.transparent
        Layout.preferredHeight: message.lineHeight
        Layout.maximumHeight: message.lineHeight
        Layout.preferredWidth: MZTheme.theme.iconSizeSmall
        Layout.rightMargin: 8
        Layout.leftMargin: 4
        Layout.alignment: Qt.AlignTop
        MZIcon {
            id: warningIcon

            source: alertColor === MZTheme.theme.red ?  "qrc:/nebula/resources/warning.svg" : "qrc:/nebula/resources/warning-dark-orange.svg"
            sourceSize.height: MZTheme.theme.iconSizeSmall
            sourceSize.width: MZTheme.theme.iconSizeSmall
            Layout.alignment: Qt.AlignVCenter
        }
    }

    MZTextBlock {
        id: message
        text: errorMessage
        color: alertColor
        Layout.fillWidth: true
    }

}
