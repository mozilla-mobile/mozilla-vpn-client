/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

VPNLinkButton  {
    fontSize: Theme.fontSizeSmallest
    Layout.preferredHeight: 18
    linkColor: Theme.greyLink
    buttonPadding: Theme.hSpacing / 2
}
