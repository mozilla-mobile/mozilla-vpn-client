/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNLinkButton  {
    fontSize: VPNTheme.theme.fontSizeSmallest
    Layout.preferredHeight: 18
    linkColor: VPNTheme.theme.greyLink
    buttonPadding: VPNTheme.theme.hSpacing / 2
}
