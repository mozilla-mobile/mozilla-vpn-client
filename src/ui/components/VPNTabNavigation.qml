/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../themes/colors.js" as Color

Item {
    id: root

    property alias tabList: tabButtons.model
    property alias stackContent: stack.children


    Rectangle {
        // grey divider
        anchors.bottom: bar.bottom

        anchors.left: parent.left
        anchors.right: parent.right
        color: Color.grey10
        height: 1
        visible: stack.children.length > 1
    }

    TabBar {
        id: bar
        width: parent.width
        visible: stack.children.length > 1
        contentHeight: stack.children.length === 1 ? 0 : 56
        background: Rectangle {
           color: "transparent"
        }

        Repeater {
            id: tabButtons

            delegate: TabButton {
                id: btn
                height: bar.height
                checkable: true

                background: Rectangle {
                    color: "transparent"

                    Rectangle {
                        height: 2
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        color: Color.purple70
                        opacity: btn.activeFocus ? 1 : 0
                        Behavior on opacity {
                            PropertyAnimation {
                                duration: 100
                            }
                        }
                    }
                }

                contentItem: VPNBoldLabel {
                    text: buttonLabel
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: btn.checked || btn.activeFocus ? Color.purple70 : btn.hovered ? Color.grey50 : Color.grey40

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
        // active tab indicator
        width: bar.currentItem.width
        height: 2
        color: Color.purple70
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

        onCurrentIndexChanged: PropertyAnimation {
                target: stack
                property: "opacity"
                from: 0
                to: 1
                duration: 200
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

