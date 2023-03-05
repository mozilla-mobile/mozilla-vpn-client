/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.Shared 1.0

Rectangle {
    id: swipeAction

    property string bgColor
    property alias content: contentLoader.sourceComponent

    color: SwipeDelegate.pressed ? Qt.darker(bgColor, 1.2) : bgColor
    height: parent.height
    width: MZTheme.theme.swipeDelegateActionWidth

    Accessible.role: Accessible.Button
    Accessible.onPressAction: SwipeDelegate.clicked()

    Keys.onSpacePressed: {
        SwipeDelegate.clicked()
    }

    Loader {
        id: contentLoader
        anchors.centerIn: parent
    }

    Rectangle {
        anchors.fill: parent
        visible: parent.activeFocus
        color: MZTheme.theme.transparent
        border.width: MZTheme.theme.focusBorderWidth
        border.color: MZTheme.theme.darkFocusBorder
    }
}
