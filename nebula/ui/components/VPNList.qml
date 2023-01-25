/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

ListView {
    id: list

    property var listName
    height:contentHeight
    Accessible.role: Accessible.List
    Accessible.name: listName
    activeFocusOnTab: true
    interactive: false // disable scrolling on list since the entire window is scrollable
    boundsBehavior: Flickable.StopAtBounds
    highlightFollowsCurrentItem: true
    Keys.onDownPressed: list.incrementCurrentIndex()
    Keys.onUpPressed: list.decrementCurrentIndex()
}
