/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: root

    property string _menuTitle: VPNl18n.SettingsTipsAndTricks

    objectName: "settingsTipsAndTricks"

    VPNFlickable {
        id: vpnFlickable

        anchors.fill: parent
        anchors.topMargin: VPNTheme.theme.menuHeight
        anchors.bottomMargin: 1

        flickContentHeight: layout.implicitHeight //parent.height - VPNTheme.theme.menuHeight
        interactive: flickContentHeight > height

        ColumnLayout {
            id: layout

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: VPNTheme.theme.vSpacing
            anchors.leftMargin: VPNTheme.theme.windowMargin
            anchors.rightMargin: VPNTheme.theme.windowMargin
        }
    }
}
