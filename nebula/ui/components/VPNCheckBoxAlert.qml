/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

RowLayout {
    property var leftMargin:  Theme.menuHeight
    property var alertColor: Theme.checkBoxWarning
    property var errorMessage

    id: turnVPNOffAlert

    visible: (VPNController.state !== VPNController.StateOff)
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.rightMargin: Theme.windowMargin
    spacing: 0
    anchors.leftMargin: Theme.windowMargin

    Rectangle {
        color: "transparent"
        Layout.preferredHeight: message.lineHeight
        Layout.maximumHeight: message.lineHeight
        Layout.preferredWidth: 14
        Layout.rightMargin: 8
        Layout.leftMargin: 4
        Layout.alignment: Qt.AlignTop
        VPNIcon {
            id: warningIcon

            source: alertColor === Theme.red ?  "qrc:/nebula/resources/warning.svg" : "qrc:/nebula/resources/warning-dark-orange.svg"
            sourceSize.height: 14
            sourceSize.width: 14
            Layout.alignment: Qt.AlignVCenter
        }
    }

    VPNTextBlock {
        id: message
        text: errorMessage
        color: alertColor
        Layout.fillWidth: true
    }

}
