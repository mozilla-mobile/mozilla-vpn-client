/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.6
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import Qt.labs.qmlmodels 1.0
import Mozilla.VPN 1.0

import "../"
import "../../themes/themes.js" as Theme
import "../../themes/colors.js" as Color

ColumnLayout {
    id: messagesContainer
    property var messages
    spacing: 8

    Component {
        id: messageItem

        RowLayout {
            spacing: 0
            visible: modelData.visible

            VPNIcon {
                id: warningIcon

                source: modelData.iconSrc
                sourceSize.height: 14
                sourceSize.width: 14
                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 4
                Layout.rightMargin: 8
                Layout.preferredWidth: 14
                Layout.preferredHeight: messageText.lineHeight
                Layout.maximumHeight: messageText.lineHeight

                // Rectangle {
                //     height: parent.height
                //     width: parent.width
                //     z: -1
                // }
            }

            VPNTextBlock {
                id: messageText

                text: modelData.message
                color: modelData.fontColor
                Layout.topMargin: 4

                // Rectangle {
                //     color: "lightgreen"
                //     height: parent.height
                //     width: parent.width
                //     z: -1
                // }
            }


            // Rectangle {
            //     color: "lightblue"
            //     height: parent.height
            //     width: parent.width
            //     z: -1
            // }
        }
    }

    DelegateChooser {
        id: messagesChooser
        role: "type"

        DelegateChoice {
            roleValue: "info"
            delegate: messageItem
        }
        DelegateChoice {
            roleValue: "warning"
            delegate: messageItem
        }
        DelegateChoice {
            roleValue: "error"
            delegate: messageItem
        }
    }

    Repeater {
        id: messagesRepeater
        delegate: messagesChooser
        model: messages
    }

    Rectangle {
        color: "white"
        height: parent.height
        width: parent.width
        z: -1
    }
}
