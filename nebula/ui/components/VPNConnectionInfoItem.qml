/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    id: infoStatusItem

    property string title
    property string subtitle
    property string iconPath
    property bool isFlagIcon: false

    spacing: VPNTheme.theme.hSpacing * 0.2

    Layout.preferredHeight: VPNTheme.theme.rowHeight
    Layout.fillWidth: true

    Item {
        id: itemIconWrapper

        height: VPNTheme.theme.iconSize * 1.5
        width: VPNTheme.theme.iconSize * 1.5

        Layout.alignment: Qt.AlignVCenter

        VPNIcon {
            id: itemIcon

            anchors.centerIn: parent
            antialiasing: true
            source: infoStatusItem.iconPath
            sourceSize.width: isFlagIcon ? VPNTheme.theme.iconSizeFlag : parent.width
            sourceSize.height: isFlagIcon ? VPNTheme.theme.iconSizeFlag : parent.height
            height: isFlagIcon ? VPNTheme.theme.iconSize : parent.height
            width: isFlagIcon ? VPNTheme.theme.iconSize : parent.width
        }
    }

    VPNBoldLabel {
        id: itemLabel
        color: VPNTheme.theme.white
        text: infoStatusItem.title
        
        Layout.alignment: Qt.AlignVCenter
        Layout.fillWidth: true
        elide: Text.ElideRight
    }

    VPNInterLabel {
        id: itemValue

        color: VPNTheme.colors.white
        text: infoStatusItem.subtitle
        horizontalAlignment: Text.AlignRight
        visible: itemValue.text !== ""
        
        Layout.alignment: Qt.AlignRight | Qt. AlignVCenter
    }
}
