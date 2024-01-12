/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0

StackView {
    id: stackView

    property var previousItem
    property int currentIndex: -1

    onBusyChanged: if (!busy && previousItem) previousItem.data.push(dimmedOverlay.createObject(stackView))

    onCurrentItemChanged: {
        //Ensures that only the top view on the stack is visible (unless transitioning)
        currentItem.StackView.visible = Qt.binding(() => { return currentItem.x >= 0 || currentItem.x + currentItem.width <= stackView.width || currentItem.StackView.status !== StackView.Inactive  })
        if (depth > 1) {
            previousItem = get(depth - 2)
            previousItem.StackView.visible = Qt.binding(() => { return previousItem.x > -stackView.width || previousItem.StackView.status !== StackView.Inactive })
        }
    }

    onDepthChanged: {
        //Destroying the dimmer overlay
        const lastIndex = currentIndex
        currentIndex = depth - 1
        if (currentIndex >= 0 && lastIndex > currentIndex) {
            previousItem.children[previousItem.children.length - 1].destroy()
        }
    }

    Component.onCompleted: function(){
        if(!currentItem && initialItem) {
            // We don't show anything right now and inital item is set,
            // On android if initialItem is anything But a component
            // it will totaly parse that into garbage values and fail
            //
            // See https://github.com/mozilla-mobile/mozilla-vpn-client/pull/2638
            console.error("Using the initialItem property does not work on some platforms. Use Component.onCompleted: stackview.push(someURI)");
            MZUtils.exitForUnrecoverableError("Setting initialItem on a StackView is illegal. See previous logs for more information.")

        }
    }

    //Pops the top most view on the stack after the swipe-to-go-back animation
    //Does not happen when popping the stack regularly
    function popAfterSlide() {
        const enterTrans = stackView.popEnter
        const popTrans = stackView.popExit
        stackView.popEnter = null
        stackView.popExit = null
        stackView.pop()
        stackView.popEnter = enterTrans
        stackView.popExit = popTrans
    }

    Component {
        id: dimmedOverlay

        Rectangle {
            width: stackView.width
            height: stackView.height + 4
            color: "black"

            opacity: (1.0 - stackView.currentItem.x / (stackView.width) ) / 8
        }
    }

    PropertyAnimation {
        id: currentItemAnimation
        target: stackView.currentItem
        property: "x"
        duration: 100

        onFinished: {
            if (stackView.currentItem.x >= stackView.width) {
                stackView.popAfterSlide()
            }
        }
    }

    PropertyAnimation {
        id: previousItemAnimation
        target: stackView.get(stackView.depth - 2)
        property: "x"
        duration: 100
    }

    MouseArea {
        id: swipeToGoBackMouseArea

        property bool isPressed: false
        property real pressedPosition
        property real mouseXOnRelease
        property real currentMouseX

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: 30
        z: 2

        enabled: MZFeatureList.get("swipeToGoBack").isSupported && stackView.depth > 1

        onPressed: {
            swipeToGoBackMouseArea.isPressed = true
            swipeToGoBackMouseArea.pressedPosition = mouseX
            updateMouseXTimer.start()
        }

        onReleased: (mouse) => {
            swipeToGoBackMouseArea.isPressed = false
            updateMouseXTimer.stop()
            swipeToGoBackMouseArea.mouseXOnRelease = mouseX

            //Inertia swipe to the right
            if(mouse.x - swipeToGoBackMouseArea.currentMouseX >= 5 && initertiaTimer.running) {
                currentItemAnimation.to = stackView.width
                previousItemAnimation.to = 0
                currentItemAnimation.start()
                previousItemAnimation.start()
                return

            }

            //Inertia swipe to the left
            if(swipeToGoBackMouseArea.currentMouseX - mouse.x >= 5 && initertiaTimer.running) {
                currentItemAnimation.to = 0
                previousItemAnimation.to = -stackView.width
                currentItemAnimation.start()
                previousItemAnimation.start()
                return
            }

            currentItemAnimation.to = swipeToGoBackMouseArea.mouseXOnRelease < (stackView.width * 0.5) ? 0 : stackView.width
            previousItemAnimation.to = swipeToGoBackMouseArea.mouseXOnRelease > (stackView.width * 0.5) ? 0 : -stackView.width
            currentItemAnimation.start()
            previousItemAnimation.start()
        }

        onMouseXChanged: {
            initertiaTimer.start()

            var offset = mouseX - swipeToGoBackMouseArea.pressedPosition

            //Percentage of the view that is NOT in the viewport to start and needs to be scrolled in
            //1 would look weird because the whole thing would need to be scrolled in
            //iOS seems to use something around .3
            const startingXPercentage = 0.3

            //Never want the view going past this point
            const maxX = 0

            //Starting value of where the view is, as it comes from off screen to on screen
            const startingX = -window.width * startingXPercentage

            //How fast we move the view on screen depends on the new mouse position in relation to where we started dragging (offset)
            //and how the rate at which we should be moving the view on screen (startingPercentage)
            const newOffset = offset * startingXPercentage

            stackView.get(stackView.depth - 2).x = Math.min(maxX, startingX + newOffset)
            stackView.get(stackView.depth - 1).x = Math.max(0, offset)
        }
    }

    Timer {
        id: updateMouseXTimer
        interval: 100
        repeat: true
        onTriggered: {
            swipeToGoBackMouseArea.currentMouseX = swipeToGoBackMouseArea.mouseX
        }
    }

    Timer {
        id: initertiaTimer
        interval: 100
    }

    Connections {
        target: MZNavigator
        function onGoBack(item) {
            if (item === stackView) {
                stackView.pop();
            }
        }
    }
}
