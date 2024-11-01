/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.Shared 1.0
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

Rectangle {
    id: swipeAction

    property string bgColor
    property alias content: contentLoader.sourceComponent

    color: SwipeDelegate.pressed ? Qt.darker(bgColor, 1.2) : bgColor
    height: parent.height
    width: MZTheme.theme.swipeDelegateActionWidth

    onActiveFocusChanged: MZUiUtils.scrollToComponent(swipeAction)

    Accessible.role: Accessible.Button
    Accessible.onPressAction: SwipeDelegate.clicked()
    Accessible.ignored: !visible

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
        color: MZTheme.colors.transparent
        border.width: MZTheme.theme.focusBorderWidth
        border.color: MZTheme.colors.darkFocusBorder
    }
}
