/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import "../themes/themes.js" as Theme

// VPNFocus
Rectangle {
    property var itemToFocus
    property var itemToAnchor
    property var focusWidth: 2

    radius: 4
    color: "transparent"
    border.width: itemToFocus.activeFocus ? focusWidth : 0
    border.color: Theme.blueFocusStroke
    anchors.fill: itemToAnchor || itemToFocus
    z: 1
}
