/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNClickableRow {
    id: root

    property var settingTitle
    property alias imageLeftSrc: icon.source
    property alias imageRightSrc: imageRight.source
    property alias imageRightMirror: imageRight.mirror
    property string fontColor: VPNTheme.theme.fontColorDark

    accessibleName: settingTitle

    anchors.left: undefined
    anchors.right: undefined

    Layout.alignment: Qt.AlignHCenter
    Layout.minimumHeight: VPNTheme.theme.rowHeight
    Layout.fillWidth: true
    canGrowVertical: true
    Layout.preferredHeight: title.lineCount > 1 ? title.implicitHeight + VPNTheme.theme.windowMargin : VPNTheme.theme.rowHeight

    RowLayout {
        id: row
        spacing: VPNTheme.theme.windowMargin
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        width: root.width - VPNTheme.theme.windowMargin

        Rectangle {
            Layout.preferredHeight: VPNTheme.theme.rowHeight
            Layout.preferredWidth: icon.width
            Layout.alignment: Qt.AlignTop
            color: VPNTheme.theme.transparent

            VPNIcon {
                id: icon
                anchors.centerIn: parent
            }
        }

        VPNBoldLabel {
            id: title
            text: settingTitle
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            Layout.fillWidth: true
            lineHeightMode: Text.FixedHeight
            lineHeight: VPNTheme.theme.labelLineHeight
            wrapMode: Text.WordWrap
            topPadding: 1
            elide: Text.ElideRight
        }

        VPNIcon {
            id: imageRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }
    }
}
