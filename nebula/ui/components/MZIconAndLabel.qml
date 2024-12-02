/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

// MZIconAndLabel
RowLayout {
    id: iconAndLabel
    property alias icon: icon.source
    property alias title: title.text
    property string fontColor: MZTheme.colors.fontColorDark
    property bool showIndicator: false

    Layout.alignment: Qt.AlignLeft

    spacing: 0

    MZIcon {
        id: icon
        Layout.alignment: Qt.AlignVCenter
    }

    MZBoldLabel {
        id: title
        color: fontColor
        // MZIconAndLabel is only used inside a MZClickableRow
        // which acts as atomic interactive control thus we want
        // to hide its content (such as MZIconAndLabel) from
        // assistive technology.
        Accessible.ignored: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        lineHeightMode: Text.FixedHeight
        lineHeight: MZTheme.theme.labelLineHeight
        leftPadding: MZTheme.theme.windowMargin
        rightPadding: MZTheme.theme.windowMargin / 2
        topPadding: 1
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        Layout.minimumHeight: MZTheme.theme.vSpacing
    }

    Rectangle {
        id: indicator
        visible: iconAndLabel.showIndicator
        color: MZTheme.colors.errorAccentLight

        radius: Layout.preferredWidth / 2
        Layout.preferredWidth: MZTheme.theme.iconSize / 2
        Layout.preferredHeight: MZTheme.theme.iconSize / 2
    }
}
