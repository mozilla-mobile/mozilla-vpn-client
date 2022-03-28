/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1
import components 0.1

ComboBox {
    property var placeholderText: ""
    property bool showInteractionStates: true
    textRole: "name"
    valueRole: "value"
    id: combo
    Layout.preferredWidth: parent.width
    Layout.preferredHeight: VPNTheme.theme.rowHeight
    currentIndex: -1
    activeFocusOnTab: true
    background: VPNInputBackground {
        z: -1
    }

    indicator: VPNIcon {
        anchors.verticalCenter: combo.verticalCenter
        anchors.right: combo.right
        anchors.rightMargin: VPNTheme.theme.windowMargin / 2
        source: "qrc:/nebula/resources/chevron.svg"
        opacity: comboPopup.visible || combo.focus ? 1 : .7
        rotation: 90
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    contentItem: VPNTextBlock {
        id: contentItem
        text: currentIndex === -1 ? placeholderText : currentText
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        anchors.left: parent.left
        anchors.leftMargin: VPNTheme.theme.windowMargin
        width: parent.width - VPNTheme.theme.windowMargin * 2
        elide: Text.ElideRight
    }

    delegate: ItemDelegate {
        id: comboDelegate
        width: parent.width
        height: Math.max(VPNTheme.theme.rowHeight, textItem.implicitHeight)
        padding: 0
        highlighted: combo.highlightedIndex === index

        contentItem: Rectangle {
            anchors.fill: parent
            height: comboDelegate.height
            color: hovered || combo.highlightedIndex === index ? VPNTheme.theme.input.highlight : VPNTheme.theme.input.backgroundColor

            Behavior on color {
                ColorAnimation {
                    duration: 100
                }
            }

            VPNTextBlock {
                id: textItem
                text: name
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: VPNTheme.theme.windowMargin / 2
                width:  parent.width - VPNTheme.theme.windowMargin * 2
                verticalAlignment: Text.AlignVCenter
                color: hovered || combo.highlightedIndex === index ? VPNTheme.theme.blue : VPNTheme.theme.fontColor

                Behavior on color {
                    ColorAnimation {
                        duration: 100
                    }
                }
            }
        }
    }

    popup: Popup {
        id: comboPopup
        y: combo.height + 12
        width: combo.width
        padding: 0
        opacity: visible ? 1 : 0

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }

        background: Item {
            Rectangle {
                id: shadowMask
                anchors.fill: bg
                visible: false
                color: "black"
                radius: VPNTheme.theme.cornerRadius
            }

            VPNDropShadow {
                source: shadowMask
                anchors.fill: shadowMask
                transparentBorder: true
                radius: 7.5
                color: "#0C0C0D"
                z: -1
                opacity: .1
                cached: true
            }
            VPNInputBackground {
                id: bg
                anchors.fill: parent
                anchors.bottomMargin: -4
                anchors.topMargin: anchors.bottomMargin
                border.color: VPNTheme.theme.transparent
            }
        }

        contentItem: ListView {
            id: listView
            implicitHeight: contentHeight
            anchors.fill: parent
            clip: true
            model: combo.popup.visible ? combo.delegateModel : null
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }

    states: [
        State {
            name: "focused"
            when: comboPopup.visible || combo.focus

            PropertyChanges {
                target: combo.contentItem
                color: VPNTheme.colors.input.focus.text
            }

            PropertyChanges {
                target: combo.background
                border.color: VPNTheme.colors.input.focus.border
                border.width: 2
            }
        },
        State {
            name: "empty"
            when: combo.currentIndex === -1
                  && !combo.hovered
                  && !comboPopup.visible

            PropertyChanges {
                target: combo.contentItem
                color: VPNTheme.colors.input.default.placeholder
            }

            PropertyChanges {
                target: combo.background
                border.color: VPNTheme.colors.input.default.border
                border.width: 1
            }
        },
        State {
            name: "emptyHovered"
            when: combo.hovered
                  && combo.currentIndex === -1
                  && !comboPopup.visible

            PropertyChanges {
                target: combo.contentItem
                color: VPNTheme.colors.input.hover.text
            }

            PropertyChanges {
                target: combo.background
                border.color: VPNTheme.colors.input.hover.border
                border.width: 1
            }
        },
        State {
            name: "filled"
            when: combo.currentIndex >= 0 && !comboPopup.visible

            PropertyChanges {
                target: combo.contentItem
                color: VPNTheme.colors.input.default.text
            }

            PropertyChanges {
                target: combo.background
                border.color: VPNTheme.colors.input.default.border
                border.width: 1
            }
        }
    ]
}
