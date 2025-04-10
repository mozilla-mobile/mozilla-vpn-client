/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.Shared 1.0
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

SwipeDelegate {
    id: swipeDelegate

    property alias content: contentLoader
    property bool hasUiStates: true
    property bool blockClose: false
    property bool isSwipeOpen: false
    property var onSwipeOpen: () => {}
    property var onSwipeClose: () => {}
    property var uiState: MZTheme.theme.uiState

    function closeSwipe() {
        if(!swipeDelegate.isSwipeOpen || overlayMouseArea.mouseX <= overlayMouseArea.pressedMouseX && !swipeDelegate.blockClose) swipeDelegate.swipe.close()
    }

    padding: 0
    clip: true
    hoverEnabled: true
    activeFocusOnTab: !blockClose
    implicitHeight: contentLoader.item.implicitHeight
    Accessible.ignored: !visible

    background: Rectangle {
        color: MZTheme.colors.bgColor
    }

    onReleased: {
        swipeDelegate.state = swipeDelegate.uiState.stateDefault
    }

    onCanceled: {
        swipeDelegate.state = swipeDelegate.uiState.stateDefault
    }

    swipe.onOpened: {
        isSwipeOpen = true
        onSwipeOpen()
    }

    swipe.onClosed: {
        isSwipeOpen = false
        onSwipeClose()
    }

    Keys.onRightPressed: {
        if(swipe.left) {
            swipe.open(SwipeDelegate.Left)
        }
    }

    Keys.onLeftPressed: {
        if(swipe.left && !blockClose) {
            swipe.close()
        }
    }

    Keys.onEnterPressed: {
         swipeDelegate.closeSwipe()
    }

    Keys.onReturnPressed: {
        swipeDelegate.closeSwipe()
    }

    onActiveFocusChanged: if(activeFocus) MZUiUtils.scrollToComponent(swipeDelegate)

    contentItem: Item {
        id: swipeDelegateContentItem

        property var backgroundColor: MZTheme.colors.iconButtonLightBackground

        width: parent.width
        height: parent.height

        Rectangle {
            id: messageBackground

            anchors.fill: parent
            opacity: 0
            visible: swipeDelegate.hasUiStates

            states: [
                State {
                    when: swipeDelegate.state === swipeDelegate.uiState.stateHovered

                    PropertyChanges {
                        target: messageBackground
                        color: swipeDelegateContentItem.backgroundColor.buttonHovered
                        opacity: 1
                    }
                },
                State {
                    when: swipeDelegate.state === swipeDelegate.uiState.statePressed

                    PropertyChanges {
                        target: messageBackground
                        color: swipeDelegateContentItem.backgroundColor.buttonPressed
                        opacity: 1
                    }
                },
                State {
                    when: swipeDelegate.state === swipeDelegate.uiState.stateDefault

                    PropertyChanges {
                        target: messageBackground
                        color: swipeDelegateContentItem.backgroundColor.defaultColor
                        opacity: 0
                    }
                }
            ]

            Behavior on opacity {
                PropertyAnimation {
                    duration: 200
                }
            }
        }

        Rectangle {
            anchors.fill: parent

            visible: swipeDelegate.activeFocus
            color: MZTheme.colors.transparent
            border.width: MZTheme.theme.focusBorderWidth
            border.color: swipeDelegateContentItem.backgroundColor.focusBorder
        }

        Loader {
            id: contentLoader
            anchors.fill: parent
        }
    }

    //Tap to close, but lose closing swipe gesture fullly interuptible control (auto closes on swipe instead)
    MouseArea {
        id: overlayMouseArea
        property real pressedMouseX

        enabled: swipeDelegate.isSwipeOpen
        anchors.fill: parent
        anchors.leftMargin: swipeDelegate.isSwipeOpen && swipe.leftItem ? swipe.leftItem.width : 0
        anchors.rightMargin: swipeDelegate.isSwipeOpen && swipe.rightItem ? swipe.rightItem.width : 0

        onMouseXChanged: if(mouseX < pressedMouseX && !blockClose) swipeDelegate.swipe.close()
        onPressed: pressedMouseX = mouseX
        onClicked: swipeDelegate.closeSwipe()
        onReleased: swipeDelegate.state = swipeDelegate.uiState.stateDefault
    }

    MZMouseArea {
        id: buttonMouseArea
        propagateComposedEvents: true
        onPressed: (mouse) => { mouse.accepted = false }
    }
}
