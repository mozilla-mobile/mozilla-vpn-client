/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import Mozilla.Shared 1.0

Rectangle {
    id: root

    //A type is a JS object containing the badgeText and badgeBackgroundColor and badgeTextColor
    property var badgeType

    //Alternatively to setting the badgeType, you can set it's properties manually
    property alias backgroundColor: root.color
    property alias text: badgeLabel.text
    property alias textColor: badgeLabel.color

    property alias badgeLabel: badgeLabel

    color: badgeType.badgeBackgroundColor
    height: badgeLabel.height
    width: badgeLabel.width
    radius: 4
    clip: true

    MZBoldInterLabel {
        id: badgeLabel

        text: badgeType.badgeText
        color: badgeType.badgeTextColor
        verticalAlignment: Text.AlignVCenter
        topPadding: MZTheme.theme.badgeVerticalPadding
        leftPadding: MZTheme.theme.badgeHorizontalPadding
        rightPadding: MZTheme.theme.badgeHorizontalPadding
        bottomPadding: MZTheme.theme.badgeVerticalPadding
        maximumLineCount: 1
        elide: Text.ElideRight
    }
}
