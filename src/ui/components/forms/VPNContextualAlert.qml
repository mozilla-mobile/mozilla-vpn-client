/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.6
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

import components 0.1
import themes 0.1

RowLayout {
    id: messageItem

    property string iconSrc
    property string fontColor

    spacing: 0
    visible: modelData.visible
    states: [
        State {
            name: "visible"
            when: visible
        },
        State {
            name: "hidden"
            when: !visible
        }
    ]
    transitions: [
        Transition {
            to: "hidden"
            SequentialAnimation {
                PropertyAnimation {
                    target: messageItem
                    property: "opacity"
                    to: 0
                    duration: 100
                }
            }
        },
        Transition {
            to: "visible"
            SequentialAnimation {
                PropertyAnimation {
                    target: messageItem
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 100
                }
            }
        }
    ]
    Layout.fillWidth: true

    VPNIcon {
        id: warningIcon

        source: iconSrc
        sourceSize.height: Theme.iconSize - Theme.focusBorderWidth
        sourceSize.width: Theme.iconSize - Theme.focusBorderWidth

        Layout.alignment: Qt.AlignTop
        Layout.leftMargin: Theme.listSpacing / 2
        Layout.maximumHeight: messageText.lineHeight
        Layout.preferredHeight: messageText.lineHeight
        Layout.preferredWidth: Theme.iconSize - Theme.focusBorderWidth
        Layout.rightMargin: Theme.listSpacing
        Layout.topMargin: 1
    }

    VPNTextBlock {
        id: messageText

        color: fontColor
        text: modelData.message
        wrapMode: Text.WordWrap

        Layout.fillWidth: true
        Layout.topMargin: Theme.listSpacing / 2
    }
}
