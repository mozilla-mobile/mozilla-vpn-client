/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Mozilla.VPN 1.0

//VPNHeaderLink
VPNLinkButton {
    property bool isLightTheme: true

    anchors {
        top: parent.top
        right: parent.right
        topMargin: VPNTheme.theme.listSpacing * 1.5
        rightMargin: VPNTheme.theme.listSpacing * 1.5
    }
    horizontalPadding: VPNTheme.theme.listSpacing * 0.5
    linkColor: isLightTheme ? VPNTheme.theme.blueButton : VPNTheme.theme.whiteButton
    width: contentItem.implicitWidth + leftPadding + rightPadding
}
