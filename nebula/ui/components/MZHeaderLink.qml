/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Mozilla.Shared 1.0

//MZHeaderLink
MZLinkButton {
    property bool isLightTheme: true

    anchors {
        top: parent.top
        right: parent.right
        topMargin: MZTheme.theme.listSpacing * 1.5
        rightMargin: MZTheme.theme.listSpacing * 1.5
    }
    horizontalPadding: MZTheme.theme.listSpacing * 0.5
    linkColor: isLightTheme ? MZTheme.theme.blueButton : MZTheme.theme.whiteButton
    width: contentItem.implicitWidth + leftPadding + rightPadding
}
