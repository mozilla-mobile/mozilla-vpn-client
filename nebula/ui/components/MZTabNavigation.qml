/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0

Item {
    id: root

    property alias tabList: tabButtons.model
    property alias stackContent: stack.children
    property variant currentTab: bar.currentItem
    property var handleTabClick: (()=> {});

    function setCurrentTabIndex(idx) {
        bar.setCurrentIndex(idx);
    }

    function selectTab(tabButton) {
        bar.currentIndex = tabButton.TabBar.index;
        // Emit the clicked signal to invoke handleTabClick, which handles the selection change
        tabButton.clicked();
    }

    Rectangle {
        // grey divider
        anchors.bottom: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: MZTheme.colors.divider
        height: 1
        visible: stack.children.length > 1
    }

    TabBar {
        id: bar
        objectName: "tabBar"
        width: root.width
        visible: stack.children.length > 1
        contentHeight: stack.children.length === 1 ? 0 : MZTheme.theme.menuHeight
        background: Rectangle {
            color: MZTheme.colors.transparent
        }

        Repeater {
            id: tabButtons

            delegate: TabButton {
                id: btn
                objectName: tabButtonId
                height: bar.contentHeight
                // Qt doesn't correctly provide the selected state for a tab on Windows, so include the selected state in the accessibility name.
                Accessible.name: ((Qt.platform.os === "windows") && (bar.currentIndex === btn.TabBar.index)) ? (MZI18n.AccessibilitySelectedAndItemName.arg(MZI18n[tabLabelStringId])) : MZI18n[tabLabelStringId]
                Accessible.ignored: !visible
                Accessible.onPressAction: selectTab(btn)
                Keys.onReturnPressed: selectTab(btn)
                Keys.onSpacePressed: selectTab(btn)

                onClicked: handleTabClick(btn)

                background: Rectangle {
                    color: MZTheme.colors.transparent

                    Rectangle {
                        height: 2
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        color: MZTheme.colors.primaryBrighter
                        opacity: btn.checked ? 1 : 0
                        Behavior on opacity {
                            PropertyAnimation {
                                duration: 100
                            }
                        }
                    }
                }

                contentItem: MZBoldLabel {
                    text: MZI18n[tabLabelStringId]
                    elide: Qt.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: btn.checked ? MZTheme.colors.primaryBrighter : btn.hovered || btn.activeFocus ? MZTheme.colors.fontColorDark : MZTheme.colors.fontColor
                    // Accessibility provided by btn's Accessible properties
                    Accessible.ignored: true

                    Behavior on color {
                        PropertyAnimation {
                            duration: 100
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        objectName: "activeTabIndicator"
        width: (bar.visible && bar.currentItem) ? bar.currentItem.width : 0
        height: 2
        color: MZTheme.colors.primaryBrighter
        anchors.bottom: bar.bottom
        x: (currentTab && currentTab.x && currentTab.ListView.view) ? currentTab.x - currentTab.ListView.view.originX : 0
        visible: stack.children.length > 1
        Behavior on x {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    StackLayout {
        id: stack
        width: parent.width
        currentIndex: bar.currentIndex
        anchors.top: bar.bottom
        height: root.height
        clip: true
        Accessible.role: Accessible.List
        Accessible.name: currentTab.Accessible.name
        Accessible.ignored: !visible

        PropertyAnimation {
            id: fadeIn
            target: stack
            property: "opacity"
            from: 0
            to: 1
            duration: 200
        }

        onCurrentIndexChanged: {
            fadeIn.start();
        }

        // pass views to this component using stackContent property

        /*
            stackContent: [
                Item { ...},
                Item { ...}
            ]
       */
    }
}
