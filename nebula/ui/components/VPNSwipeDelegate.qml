import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

SwipeDelegate {
    id: swipeDelegate

    property alias content: contentLoader
    property bool isSwipeOpen: false
    property var onSwipeOpen: () => {}
    property var onSwipeClose: () => {}
    property var uiState: VPNTheme.theme.uiState

    padding: 0
    clip: true
    hoverEnabled: true
    activeFocusOnTab: true

    background: Rectangle {
        color: VPNTheme.theme.bgColor
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

    contentItem: Item {
        id: swipeDelegateContentItem

        property var backgroundColor: VPNTheme.theme.iconButtonLightBackground

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
            color: VPNTheme.theme.transparent
            border.width: VPNTheme.theme.focusBorderWidth
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

    VPNMouseArea {
        id: buttonMouseArea
        propagateComposedEvents: true
        onPressed: (mouse) => {
                       mouse.accepted = false
                   }
    }
}
