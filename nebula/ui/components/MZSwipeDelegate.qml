/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.Shared 1.0
import utils 0.1

SwipeDelegate {
    id: swipeDelegate

    property alias content: contentLoader
    property bool isSwipeOpen: false
    property var onSwipeOpen: () => {}
    property var onSwipeClose: () => {}
    property var uiState: MZTheme.theme.uiState

    padding: 0
    clip: true
    hoverEnabled: true
    activeFocusOnTab: true

    background: Rectangle {
        color: MZTheme.theme.bgColor
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

    onActiveFocusChanged: if(activeFocus) MZUtils.scrollToComponent(swipeDelegate)

    contentItem: Item {
        id: swipeDelegateContentItem

        property var backgroundColor: MZTheme.theme.iconButtonLightBackground

        width: parent.width
        height: parent.height

        Rectangle {
            id: messageBackground

            anchors.fill: parent
            opacity: 0

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

            Behavior on color {
                ColorAnimation {
                    target: messageBackground
                    duration: 200
                }
            }
        }

        Rectangle {
            anchors.fill: parent

            visible: swipeDelegate.activeFocus
            color: MZTheme.theme.transparent
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
        property real pressedMouseX

        enabled: swipeDelegate.isSwipeOpen
        anchors.fill: parent
        anchors.leftMargin: swipeDelegate.isSwipeOpen && swipe.leftItem ? swipe.leftItem.width : 0
        anchors.rightMargin: swipeDelegate.isSwipeOpen && swipe.rightItem ? swipe.rightItem.width : 0

        onMouseXChanged: if(mouseX < pressedMouseX) swipeDelegate.swipe.close()
        onPressed: pressedMouseX = mouseX
        onClicked: if(!swipeDelegate.isSwipeOpen || mouseX <= pressedMouseX) swipeDelegate.swipe.close()
        onReleased: swipeDelegate.state = swipeDelegate.uiState.stateDefault
    }

    MZMouseArea {
        id: buttonMouseArea
        propagateComposedEvents: true
        onPressed: (mouse) => {
                       mouse.accepted = false
                   }
    }
}
