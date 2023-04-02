/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.Shared.qmlcomponents 1.0
import components 0.1
import components.forms 0.1

Item {
    id: root

    MZMenu {
        id: menu
        // Do not translate this string!
        title: "Messages - REMOVE ME"
        _menuOnBackClicked: () => getHelpStackView.pop()
    }

    MZFlickable {
        id: vpnFlickable
        flickContentHeight: messagessHolder.implicitHeight
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: MZTheme.theme.windowMargin

            spacing: MZTheme.theme.windowMargin
            id: messagessHolder

            MZFilterProxyModel {
                id: messagesModel
                source: MZAddonManager
                filterCallback: obj => !!obj.addon.as(MZAddon.TypeMessage)
            }

            Text {
               text: "Unread messages: " + MZAddonManager.reduce((addon, initialValue) => initialValue + (!!addon.as(MZAddon.TypeMessage) ? 1 : 0), 0);
            }

            Repeater {
                model: messagesModel
                delegate: MZCheckBoxRow {
                    property var addonMessage: addon.as(MZAddon.TypeMessage)

                    // I'm too lazy to create a proper view.
                    function showMessageContent(addonMessage) {
                        const list = [];
                        list.push("Translate title: " + addonMessage.title);
                        list.push("Is read: " + addonMessage.isRead);
                        list.push("Blocks: " + addonMessage.composer.blocks.length);
                        return list.join("\n");
                    }

                    showDivider: false
                    labelText: addonMessage.id
                    subLabelText: showMessageContent(addonMessage)
                    // Only enable the list on features where devModeEnable has any impact
                    enabled: true
                    Layout.minimumHeight: MZTheme.theme.rowHeight * 1.5

                    onClicked: {
                       if (addonMessage.isRead) {
                         addonMessage.dismiss();
                       } else {
                         addonMessage.markAsRead();
                       }
                    }
                }
            }
        }
    }
}
