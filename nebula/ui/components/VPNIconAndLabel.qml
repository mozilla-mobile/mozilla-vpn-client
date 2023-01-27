/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

// VPNIconAndLabel
RowLayout {
    id: iconAndLabel
    property alias icon: icon.source
    property alias title: title.text
    property string fontColor: VPNTheme.theme.fontColorDark
    property bool showIndicator: false

    Layout.alignment: Qt.AlignLeft

    spacing: 0

    VPNIcon {
        id: icon
        Layout.alignment: Qt.AlignVCenter
    }

    VPNBoldLabel {
        id: title
        color: fontColor
        // VPNIconAndLabel is only used inside a VPNClickableRow
        // which acts as atomic interactive control thus we want
        // to hide its content (such as VPNIconAndLabel) from
        // assistive technology.
        Accessible.ignored: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        lineHeightMode: Text.FixedHeight
        lineHeight: VPNTheme.theme.labelLineHeight
        leftPadding: VPNTheme.theme.windowMargin
        rightPadding: VPNTheme.theme.windowMargin / 2
        topPadding: 1
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        Layout.minimumHeight: VPNTheme.theme.vSpacing
    }

    Rectangle {
        id: indicator
        visible: iconAndLabel.showIndicator
        color: VPNTheme.colors.error.default

        radius: Layout.preferredWidth / 2
        Layout.preferredWidth: VPNTheme.theme.iconSize / 2
        Layout.preferredHeight: VPNTheme.theme.iconSize / 2
    }
}
