/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

Item {
    id: root

    VPNMenu {
        id: menu
        // Do not translate this string!
        title: "Theme List"
        isSettingsView: false
    }

    VPNFlickable {
        id: vpnFlickable
        flickContentHeight: themeListHolder.height + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: VPNTheme.theme.windowMargin

            spacing: VPNTheme.theme.windowMargin
            id: themListHolder

            ButtonGroup {
                id: radioButtonGroup
            }

            Repeater {
                model: VPNTheme
                delegate: VPNRadioDelegate {
                    radioButtonLabelText: name
                    accessibleName: name
                    checked: VPNTheme.currentTheme === name
                    onClicked: VPNTheme.currentTheme = name
                }
            }
        }
    }
}
