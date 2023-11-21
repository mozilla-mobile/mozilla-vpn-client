/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.6
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1

RowLayout {
    id: messageItem

    property string iconSrc
    property string fontColor

    Layout.preferredWidth: parent.width
    Layout.fillWidth: true
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
            ScriptAction { script: onShowCompleted(); }
        }
    ]

    function onShowCompleted() {
        if (!messageText.Accessible.ignored)
            MZAccessibleNotification.notify(messageText, messageText.text);
    }

    MZIcon {
        id: warningIcon

        source: iconSrc
        sourceSize.height: MZTheme.theme.iconSize - MZTheme.theme.focusBorderWidth
        sourceSize.width: MZTheme.theme.iconSize - MZTheme.theme.focusBorderWidth

        Layout.alignment: Qt.AlignTop
        Layout.leftMargin: MZTheme.theme.listSpacing / 2
        Layout.maximumHeight: messageText.lineHeight
        Layout.preferredHeight: messageText.lineHeight
        Layout.preferredWidth: MZTheme.theme.iconSize - MZTheme.theme.focusBorderWidth
        Layout.rightMargin: MZTheme.theme.listSpacing
        Layout.topMargin: 1
        visible: messageText.text && messageText.text.length !== 0;
    }

    MZTextBlock {
        id: messageText
        Accessible.role: Accessible.StaticText

        color: fontColor
        text: modelData.message
        wrapMode: Text.WordWrap
        width: undefined
        Layout.fillWidth: true
        Layout.topMargin: MZTheme.theme.listSpacing / 2
    }
}
