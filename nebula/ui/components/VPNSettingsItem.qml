/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNClickableRow {
    id: root

    property var settingTitle
    property var imageLeftSrc
    property var imageRightSrc
    property bool showIndicator: false
    property string fontColor: VPNTheme.theme.fontColorDark

    accessibleName: settingTitle

    anchors.left: undefined
    anchors.right: undefined

    Layout.alignment: Qt.AlignHCenter
    Layout.minimumHeight: VPNTheme.theme.rowHeight
    Layout.preferredWidth: parent.width

    RowLayout {
        id: row
        spacing: 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: VPNTheme.theme.windowMargin / 2
        anchors.rightMargin: VPNTheme.theme.windowMargin / 2
        anchors.verticalCenter: parent.verticalCenter

        VPNIconAndLabel {
            id: iconAndLabel
            icon: imageLeftSrc
            title: settingTitle
            fontColor: root.fontColor
            showIndicator: root.showIndicator
        }

        VPNIcon {
            id: imageRight
            source: imageRightSrc
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }
    }
}
