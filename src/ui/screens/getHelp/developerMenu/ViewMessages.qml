/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1
import components.forms 0.1

Item {
    id: root

    VPNMenu {
        id: menu
        // Do not translate this string!
        title: "Messages - REMOVE ME"
        _menuOnBackClicked: () => getHelpStackView.pop()
    }

    VPNFlickable {
        id: vpnFlickable
        flickContentHeight: messagessHolder.height + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: VPNTheme.theme.windowMargin

            spacing: VPNTheme.theme.windowMargin
            id: messagessHolder

            VPNFilterProxyModel {
                id: messagesModel
                source: VPNAddonManager
                filterCallback: obj => obj.addon.type === "message"
            }

            Text {
               text: "Unread messages: " + VPNAddonManager.reduce((addon, initialValue) => initialValue + (addon.type === "message" ? 1 : 0), 0);
            }

            Repeater {
                model: messagesModel
                delegate: VPNCheckBoxRow {
                    // I'm too lazy to create a proper view.
                    function showMessageContent(addon) {
                        const list = [];
                        list.push("Translate title: " + addon.title);
                        list.push("Is read: " + addon.isRead);
                        list.push("Blocks: " + addon.composer.blocks.length);
                        return list.join("\n");
                    }

                    showDivider: false
                    labelText: addon.id
                    subLabelText: showMessageContent(addon)
                    showAppImage: false
                    // Only enable the list on features where devModeEnable has any impact
                    enabled: true
                    Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5

                    onClicked: {
                       if (addon.isRead) {
                         addon.dismiss();
                       } else {
                         addon.markAsRead();
                       }
                    }
                }
            }
        }
    }
}
