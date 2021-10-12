/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import themes 0.1

Rectangle {

    id: focusInnerBorder

    color: "transparent"
    anchors.fill: parent
    radius: parent.radius
    border.width: Theme.focusBorderWidth
    border.color: colorScheme.focusBorder
    opacity: itemToFocus.activeFocus ? 1 : 0
    antialiasing: true
}
