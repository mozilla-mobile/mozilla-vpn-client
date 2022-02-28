/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Qt.labs.qmlmodels 1.0

import Mozilla.VPN 1.0

ColumnLayout {
    property string listHeader
    property variant listModel: ({})

    id: root

    spacing: 0

    VPNTextBlock {
        Layout.fillWidth: true
        Layout.bottomMargin: VPNTheme.theme.listSpacing

        color: VPNTheme.colors.white
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
                sourceSize.height: VPNTheme.theme.iconSize * 1.5
                sourceSize.width: VPNTheme.theme.iconSize * 1.5
            }
            VPNTextBlock {
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 2
                color: VPNTheme.colors.grey20
                text: title
                wrapMode: Text.WordWrap
            }
        }
    }

    Repeater {
        id: checkmarkListRepeater
        delegate: checkMarkItem
        model: checkmarkListModel
    }

}