/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0        
        
        
RowLayout {
    id: linkRow
    Layout.fillWidth: true
    Layout.rightMargin: MZTheme.theme.windowMargin
    Layout.leftMargin: MZTheme.theme.windowMargin
    property alias title: label.text
    property alias subLabelText: subLabel.text

    signal clicked()

    spacing: 16
    Accessible.name: title

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop

        spacing: 0

        MZInterLabel {
            id: label
            Layout.fillWidth: true
            text: ""
            color: MZTheme.colors.fontColorDark
            horizontalAlignment: Text.AlignLeft
            visible: !!text.length
            Layout.topMargin: 10
        }

        MZTextBlock {
            id: subLabel

            Layout.fillWidth: true
            text: ""
            font.pixelSize: MZTheme.theme.fontSizeSmall
            visible: !!text.length
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
        }

        Rectangle {
            id: divider

            Layout.topMargin: MZTheme.theme.toggleRowDividerSpacing
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            color: MZTheme.colors.divider
        }
    }

    MZIconButton {
        id: icon
        Layout.alignment: Qt.AlignTop
        Layout.preferredHeight: 50
        Layout.preferredWidth: 50
        buttonColorScheme: MZTheme.colors.clickableRow
        accessibleName: title
        MZIcon {
            source: "qrc:/nebula/resources/externalLink.svg"
            anchors.centerIn: parent
        }
        onClicked: linkRow.clicked()
    }
}
