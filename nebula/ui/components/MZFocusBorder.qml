/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0

Rectangle {

    id: focusInnerBorder

    color: MZTheme.colors.transparent
    anchors.fill: parent
    radius: parent.radius
    border.width: MZTheme.theme.focusBorderWidth
    border.color: colorScheme.focusBorder
    opacity: itemToFocus.activeFocus ? 1 : 0
    antialiasing: true
}
