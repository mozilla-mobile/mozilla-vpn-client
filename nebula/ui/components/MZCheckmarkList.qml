/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

ColumnLayout {
    property string listHeader
    property variant listModel: ({})

    id: root

    spacing: MZTheme.theme.listSpacing

    MZTextBlock {
        Layout.fillWidth: true

        color: MZTheme.colors.white
        text: root.listHeader
        wrapMode: Text.WordWrap
    }

    Component {
        id: checkMarkItem

        Row {
            Layout.fillWidth: true

            Image {
                fillMode: Image.PreserveAspectFit
                source: type === "checkmark"
                    ? "qrc:/nebula/resources/checkmark-green.svg"
                    : "qrc:/nebula/resources/dropdown-closed.svg"
                sourceSize.height: MZTheme.theme.iconSize * 1.5
                sourceSize.width: MZTheme.theme.iconSize * 1.5
            }
            MZTextBlock {
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 2
                color: MZTheme.colors.grey20
                text: title
                wrapMode: Text.WordWrap
            }
        }
    }

    Repeater {
        id: checkmarkListRepeater
        delegate: checkMarkItem
        model: listModel
    }

}
