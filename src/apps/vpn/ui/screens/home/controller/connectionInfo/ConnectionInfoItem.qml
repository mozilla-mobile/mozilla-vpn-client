/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1
import telemetry 0.30


RowLayout {
    id: infoStatusItem

    property string title
    property string subtitle
    property string iconPath
    property bool isFlagIcon: false

    spacing: MZTheme.theme.hSpacing * 0.2

    Layout.preferredHeight: MZTheme.theme.rowHeight
    Layout.fillWidth: true

    Item {
        id: itemIconWrapper

        height: MZTheme.theme.iconSize * 1.5
        width: MZTheme.theme.iconSize * 1.5

        Layout.alignment: Qt.AlignVCenter

        MZIcon {
            id: itemIcon

            anchors.centerIn: parent
            antialiasing: true
            source: infoStatusItem.iconPath
            sourceSize.width: isFlagIcon ? MZTheme.theme.iconSizeFlag : parent.width
            sourceSize.height: isFlagIcon ? MZTheme.theme.iconSizeFlag : parent.height
            height: isFlagIcon ? MZTheme.theme.iconSize : parent.height
            width: isFlagIcon ? MZTheme.theme.iconSize : parent.width
        }
    }

    MZBoldLabel {
        id: itemLabel
        color: MZTheme.theme.white
        text: infoStatusItem.title
        
        Layout.alignment: Qt.AlignVCenter
        Layout.fillWidth: true
        elide: Text.ElideRight
    }

    MZInterLabel {
        id: itemValue

        color: MZTheme.colors.white
        text: infoStatusItem.subtitle
        horizontalAlignment: Text.AlignRight
        visible: itemValue.text !== ""
        
        Layout.alignment: Qt.AlignRight | Qt. AlignVCenter
    }
}
