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
        title: "Guides - REMOVE ME"
        isSettingsView: false
    }

    VPNFlickable {
        id: vpnFlickable
        flickContentHeight: guidesHolder.height + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: VPNTheme.theme.windowMargin

            spacing: VPNTheme.theme.windowMargin
            id: guidesHolder

            Repeater {
                model: VPNGuide
                delegate: VPNCheckBoxRow {
                    // I'm too lazy to create a proper view.
                    function showGuideContent(guide) {
                        const list = [];
                        list.push("Translate title: " + VPNl18n[guide.id]);
                        list.push("Image: " + guide.image);
                        list.push("Blocks: " + guide.blocks.length);

                        for (let block of guide.blocks) {
                            switch (block.type) {
                                case VPNGuide.GuideBlockTypeTitle:
                                    list.push("Block title: " + VPNl18n[block.id]);
                                    break;
                                case VPNGuide.GuideBlockTypeText:
                                    list.push("Block text: " + VPNl18n[block.id]);
                                    break;
                                case VPNGuide.GuideBlockTypeOrderedList:
                                    list.push("Block list:");
                                    block.subBlockIds.forEach((subBlockId, pos) => {
                                        list.push(pos + ". " + VPNl18n[subBlockId]);
                                    });
                                    break;
                                case VPNGuide.GuideBlockTypeUnorderedList:
                                    list.push("Block list:");
                                    block.subBlockIds.forEach(subBlockId => {
                                        list.push("- " + VPNl18n[subBlockId]);
                                    });
                                    break;
                            }
                        }

                        return list.join("\n");
                    }

                    showDivider: false
                    labelText: guide.id
                    subLabelText: showGuideContent(guide)
                    showAppImage: false
                    // Only enable the list on features where devModeEnable has any impact
                    enabled: true
                    Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5
                }
            }
        }
    }
}
