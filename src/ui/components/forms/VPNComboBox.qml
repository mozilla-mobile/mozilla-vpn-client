/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../../themes/themes.js" as Theme
import "../"

ComboBox {
    property var placeholderText: ""
    textRole: "name"
    valueRole: "value"
    id: combo
    Layout.preferredWidth: parent.width
    Layout.preferredHeight: Theme.rowHeight
    currentIndex: -1
    activeFocusOnTab: true
    background: VPNInputBackground {
        z: -1
    }

    indicator: VPNIcon {
        anchors.verticalCenter: combo.verticalCenter
        anchors.right: combo.right
        anchors.rightMargin: Theme.windowMargin / 2
        source: "qrc:/ui/resources/chevron.svg"
        opacity: comboPopup.visible || combo.focus ? 1 : .7
        rotation: 90
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    contentItem: VPNTextBlock {
        text: currentIndex === -1 ? placeholderText : currentText
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        anchors.left: parent.left
        anchors.leftMargin: Theme.windowMargin
        width: parent.width - Theme.windowMargin * 2
        elide: Text.ElideRight
    }

    delegate: ItemDelegate {
        id: comboDelegate
        width: parent.width
        height: Math.max(Theme.rowHeight, textItem.implicitHeight)
        padding: 0
        highlighted: combo.highlightedIndex === index

        contentItem: Rectangle {
            anchors.fill: parent
            height: comboDelegate.height
            color: hovered || combo.highlightedIndex === index ? Theme.input.highlight : Theme.input.backgroundColor

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
                anchors.leftMargin: Theme.windowMargin / 2
                width:  parent.width - Theme.windowMargin * 2
                verticalAlignment: Text.AlignVCenter
                color: hovered || combo.highlightedIndex === index ? Theme.blue : Theme.fontColor

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

        contentItem: ListView {
            id: listView
            implicitHeight: contentHeight
            anchors.fill: parent
            clip: true
            model: combo.popup.visible ? combo.delegateModel : null
            ScrollIndicator.vertical: ScrollIndicator {}
        }


        background: VPNInputBackground {
            id: bg
            anchors.fill: parent
            anchors.bottomMargin: -4
            anchors.topMargin: anchors.bottomMargin
            border.color: "transparent"
            DropShadow {
                source: bg
                anchors.fill:bg
                samples: 16
                transparentBorder: true
                radius: 7.5
                color: "#0C0C0D"
                z: -1
                opacity: .2
                cached: true
            }

        }
    }
}
