/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

Item {
    id: root

    property alias tabList: tabButtons.model
    property alias stackContent: stack.children
    property variant currentTab: bar.currentItem
    property var handleTabClick: (()=> {});

    function setCurrentTabIndex(idx) {
        bar.setCurrentIndex(idx);
    }

    Rectangle {
        // grey divider
        anchors.bottom: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: VPNTheme.colors.grey10
        height: 1
        visible: stack.children.length > 1
    }

    TabBar {
        id: bar
        objectName: "tabBar"
        width: parent.width
        visible: stack.children.length > 1
        contentHeight: stack.children.length === 1 ? 0 : VPNTheme.theme.menuHeight
        background: Rectangle {
            color: "transparent"
        }

        Repeater {
            id: tabButtons

            delegate: TabButton {
                id: btn
                objectName: tabButtonId
                height: bar.contentHeight
                width: stack.children.length > 0 ? (bar.width / stack.children.length) : bar.width
                onClicked: handleTabClick(btn)

                background: Rectangle {
                    color: "transparent"

                    Rectangle {
                        height: 2
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        color: VPNTheme.colors.purple70
                        opacity: btn.activeFocus ? 1 : 0
                        Behavior on opacity {
                            PropertyAnimation {
                                duration: 100
                            }
                        }
                    }
                }

                contentItem: VPNBoldLabel {
                    text: VPNl18n[tabLabelStringId]
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: btn.checked || btn.activeFocus ? VPNTheme.colors.purple70 : btn.hovered ? VPNTheme.colors.grey50 : VPNTheme.colors.grey40

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
        width: bar.currentItem.width
        height: 2
        color: VPNTheme.colors.purple70
        anchors.bottom: bar.bottom
        x: bar.currentItem.x
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
        height: root.height - bar.contentHeight
        clip: true


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
