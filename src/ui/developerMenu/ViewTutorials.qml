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
        title: "Tutorials - REMOVE ME"
    }

    VPNFlickable {
        id: vpnFlickable
        flickContentHeight: tutorialsHolder.height + 100
        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout {
            anchors.top:  parent.top
            anchors.left:  parent.left
            anchors.leftMargin: VPNTheme.theme.windowMargin

            spacing: VPNTheme.theme.windowMargin
            id: tutorialsHolder

            Repeater {
                model: VPNTutorial
                delegate: VPNCheckBoxRow {
                    // I'm too lazy to create a proper view.
                    function showTutorialContent(tutorial) {
                        const list = [];
                        list.push("Translate title: " + VPNl18n[tutorial.titleId]);
                        list.push("Translate subtitle: " + VPNl18n[tutorial.subtitleId]);
                        list.push("Image: " + tutorial.image);
                        list.push("Highlighted: " + tutorial.highlighted + "(" + (tutorial === VPNTutorial.highlightedTutorial ? "true" : "false")  + ")");

                        return list.join("\n");
                    }

                    showDivider: false
                    labelText: tutorial.id
                    subLabelText: showTutorialContent(tutorial)
                    showAppImage: false
                    // Only enable the list on features where devModeEnable has any impact
                    enabled: true
                    Layout.minimumHeight: VPNTheme.theme.rowHeight * 1.5

                    onClicked: {
                       VPNTutorial.play(tutorial);
                       VPNCloseEventHandler.removeAllStackViews();
                    }
                }
            }
        }
    }
}
