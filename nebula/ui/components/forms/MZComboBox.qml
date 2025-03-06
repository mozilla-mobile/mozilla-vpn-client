/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import compat 0.1
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

ComboBox {
    property string placeholderText: ""
    property bool showInteractionStates: true
    textRole: "name"
    valueRole: "value"
    id: combo
    Layout.preferredWidth: parent.width
    Layout.preferredHeight: MZTheme.theme.rowHeight
    currentIndex: -1
    activeFocusOnTab: true
    Accessible.name: contentItem.text
    background: MZInputBackground {
        z: -1
    }

    onActiveFocusChanged: {
        if (focus && vpnFlickable && typeof(vpnFlickable.ensureVisible) !== "undefined")
            vpnFlickable.ensureVisible(combo)
    }

    indicator: MZIcon {
        anchors.verticalCenter: combo.verticalCenter
        anchors.right: combo.right
        anchors.rightMargin: MZTheme.theme.windowMargin / 2
        source: MZAssetLookup.getImageSource("Chevron")
        mirror: MZLocalizer.isRightToLeft
        opacity: comboPopup.visible || combo.focus ? 1 : .7
        rotation: 90
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    contentItem: MZTextBlock {
        id: contentItem
        text: currentIndex === -1 ? placeholderText : currentText
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        anchors.left: parent.left
        anchors.leftMargin: MZTheme.theme.windowMargin
        width: parent.width - MZTheme.theme.windowMargin * 2
        elide: Text.ElideRight
    }

    delegate: ItemDelegate {
        id: comboDelegate
        width: parent.width
        height: Math.max(MZTheme.theme.rowHeight, textItem.implicitHeight)
        padding: 0
        highlighted: combo.highlightedIndex === index

        contentItem: Rectangle {
            anchors.fill: parent
            height: comboDelegate.height
            color: hovered || combo.highlightedIndex === index ? MZTheme.colors.inputHighlight : MZTheme.colors.bgColorStronger

            Behavior on color {
                ColorAnimation {
                    duration: 100
                }
            }

            MZTextBlock {
                id: textItem
                text: name
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: MZTheme.theme.windowMargin / 2
                width:  parent.width - MZTheme.theme.windowMargin * 2
                verticalAlignment: Text.AlignVCenter
                color: hovered || combo.highlightedIndex === index ? MZTheme.colors.normalButton.defaultColor : MZTheme.colors.fontColor

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
                radius: MZTheme.theme.cornerRadius
            }

            MZDropShadow {
                source: shadowMask
                anchors.fill: shadowMask
                transparentBorder: true
                radius: 7.5
                color: MZTheme.colors.dropShadow
                z: -1
                opacity: .1
                cached: true
            }
            MZInputBackground {
                id: bg
                anchors.fill: parent
                anchors.bottomMargin: -4
                anchors.topMargin: anchors.bottomMargin
                border.color: MZTheme.colors.transparent
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
                color: MZTheme.colors.inputState.focus.text
            }

            PropertyChanges {
                target: combo.background
                border.color: MZTheme.colors.inputState.focus.border
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
                color: MZTheme.colors.inputState.default.placeholder
            }

            PropertyChanges {
                target: combo.background
                border.color: MZTheme.colors.inputState.default.border
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
                color: MZTheme.colors.inputState.hover.text
            }

            PropertyChanges {
                target: combo.background
                border.color: MZTheme.colors.inputState.hover.border
                border.width: 1
            }
        },
        State {
            name: "filled"
            when: combo.currentIndex >= 0 && !comboPopup.visible

            PropertyChanges {
                target: combo.contentItem
                color: MZTheme.colors.inputState.default.text
            }

            PropertyChanges {
                target: combo.background
                border.color: MZTheme.colors.inputState.default.border
                border.width: 1
            }
        }
    ]

    Connections {
        target: window
        function onScreenClicked(x, y) {
            if(combo.focus) window.removeFocus(combo, x, y)
        }
    }
}
