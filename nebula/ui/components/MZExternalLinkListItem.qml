/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

// MZExternalLinkListItem
MZClickableRow {
    property alias title: title.text
    property alias subLabelText: subLabel.text
    property string iconSource:  "qrc:/nebula/resources/externalLink.svg"
    property alias iconMirror: icon.mirror

    backgroundColor: MZTheme.theme.clickableRowBlue

    RowLayout {
        // these next 3 anchors are messing stuff up b/c I'm in a layout
        anchors.fill: parent
        anchors.leftMargin: MZTheme.theme.windowMargin / 2
        anchors.rightMargin: MZTheme.theme.windowMargin / 2

        ColumnLayout {
            // height: 40 - this seemingly has done nothing
            Layout.fillWidth: true
            // Layout.height: 40
            // implicitHeight
            // Layout.minimumHeight: 50
            spacing: 1

            MZBoldLabel {
                id: title
                Layout.fillWidth: true
            }

            MZTextBlock {
                id: subLabel
                Layout.fillWidth: true

                font.pixelSize: MZTheme.theme.fontSizeSmall
                visible: !!text.length
                wrapMode: Text.Wrap
            }
        }

        Item {
            Layout.fillWidth: true
        }

        MZIcon {
            id: icon
            source: iconSource
        }

    }

}
