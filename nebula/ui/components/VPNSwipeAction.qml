import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

Rectangle {
    id: swipeAction

    property string bgColor
    property alias content: contentLoader.sourceComponent

    color: SwipeDelegate.pressed ? Qt.darker(bgColor, 1.2) : bgColor
    height: parent.height
    width: VPNTheme.theme.swipeDelegateActionWidth

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
        color: VPNTheme.theme.transparent
        border.width: VPNTheme.theme.focusBorderWidth
        border.color: VPNTheme.theme.darkFocusBorder
    }
}
