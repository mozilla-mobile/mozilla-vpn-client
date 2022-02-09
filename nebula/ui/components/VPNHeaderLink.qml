/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

//VPNHeaderLink
VPNLinkButton {
    property bool isLightTheme: true

    anchors.top: parent.top
    anchors.right: parent.right
    anchors.topMargin: 12
    anchors.rightMargin: 12
    horizontalPadding: 4
    linkColor: isLightTheme ? VPNTheme.theme.blueButton : VPNTheme.theme.whiteButton
}
