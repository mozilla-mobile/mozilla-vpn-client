/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0

Flickable {
    id: vpnFlickable

    property var flickContentHeight
    property bool contentExceedsHeight: height < contentHeight
    property bool hideScollBarOnStackTransition: false
    property bool addNavbarHeightOffset: navbar.visible

    //This property should be true if the flickable appears behind the main navbar
    interactive: contentHeight > height || contentY > 0
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    opacity: 0

    onFlickContentHeightChanged: {
        recalculateContentHeight()
    }

    onHeightChanged: {
        recalculateContentHeight()
    }

    Component.onCompleted: {
        opacity = 1;
        if (Qt.platform.os === "windows") {
            maximumFlickVelocity = 700;
        }
    }

    ScrollBar.vertical: ScrollBar {
        property var scrollBarWidth: Qt.platform.os === "osx" ? 6 : 10
        id: scrollBar

        Accessible.ignored: true
        hoverEnabled: true
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: Qt.platform.os === "osx" ? 2 : 0

        background: Rectangle {
            color: MZTheme.theme.transparent
            anchors.fill: parent
        }

        contentItem: Rectangle {
            color: MZTheme.colors.grey40
            width: scrollBar.scrollBarWidth
            implicitWidth: scrollBar.scrollBarWidth
            radius: scrollBar.scrollBarWidth

            opacity: scrollBar.pressed ? 0.5 :
                   scrollBar.interactive && scrollBar.hovered ? 0.4 : 0.3
            Behavior on opacity {
                PropertyAnimation {
                    duration: 100
                }
            }
        }

        font.pixelSize: 0 /* QTBUG-96733 workaround */
        implicitWidth: scrollBarWidth
        width: scrollBarWidth
        minimumSize: 0

        opacity: hideScollBarOnStackTransition && (vpnFlickable.StackView.status !== StackView.Active) ? 0 : 1
        visible: contentExceedsHeight && vpnFlickable.interactive

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    function ensureVisible(item) {
        if(ensureVisAnimation.running) {
            ensureVisAnimation.stop()
        }

        const yPosition = item.mapToItem(contentItem, 0, 0).y
        if (!contentExceedsHeight || item.skipEnsureVisible || yPosition < 0) {
            return
        }

        const isItemBehindNavbar = item.mapToItem(window.contentItem, 0, 0).y + item.height > window.height - MZTheme.theme.navBarHeightWithMargins
        const isItemOffScreen = item.mapToItem(window.contentItem, 0, 0).y + item.height > window.height
        const buffer = vpnFlickable.addNavbarHeightOffset && isItemBehindNavbar ? MZTheme.theme.navBarHeightWithMargins : MZTheme.theme.contentBottomMargin
        const itemHeight = Math.max(item.height, MZTheme.theme.rowHeight) + buffer
        let destinationY

        //When focusing on an item either behind the navbar and/or that is partially or fully off screen in the downward direction
        //scroll to it so that it is visible
        if((vpnFlickable.addNavbarHeightOffset && isItemBehindNavbar) || (!vpnFlickable.addNavbarHeightOffset && isItemOffScreen)) {
            const scrollDistance = item.mapToItem(window.contentItem, 0, 0).y + item.height - (window.height - buffer)

            destinationY = contentY + scrollDistance
        }
        else {
            //When focusing on an item that is off screen in the upward direction
            if (yPosition < vpnFlickable.contentY) {
                destinationY = yPosition - buffer
            }
        }

        if (typeof(destinationY) === "undefined") return;

        ensureVisAnimation.to = destinationY
        ensureVisAnimation.start()

        return
    }

    function recalculateContentHeight() {
        //Absolute y coordinate position of the scroll view
        const absoluteYPosition = mapToItem(window.contentItem, 0, 0).y
        //Portion of the screen that a view's contents can reside without interfering with the navbar
        const contentSpace = window.height - MZTheme.theme.navBarHeightWithMargins

        //Checks if the flickable AND it's content interferes with the navbar area (bottom 146px for iOS, bottom 128px for other platforms)
        //If it does, we pad the flickable's contentHeight with whatever bottom padding is needed so that there is always 48px (aka theme.navBarTopMargin)
        //between the bottom of the flickable's content and the navbar
        if (vpnFlickable.addNavbarHeightOffset && absoluteYPosition + height >= contentSpace
                && flickContentHeight + absoluteYPosition >= contentSpace) {
            vpnFlickable.contentHeight = flickContentHeight + (flickContentHeight >= height ? MZTheme.theme.navBarHeightWithMargins : (absoluteYPosition + flickContentHeight) - contentSpace + (height - flickContentHeight))
        }
        //If the navbar isn't visible, or the flickable's content does not interfere with the navbar area, don't worry about adding any padding
        else {
            vpnFlickable.contentHeight = flickContentHeight
        }
    }

    NumberAnimation on contentY {
        id: ensureVisAnimation

        duration: 300
        easing.type: Easing.OutQuad
    }

    PropertyAnimation on opacity {
        duration: 200
    }

    MouseArea {

        anchors.fill: parent
        propagateComposedEvents: true

        onPressed: mouse => {
            if (window.activeFocusItem &&
                window.activeFocusItem.forceBlurOnOutsidePress &&
                (Qt.platform.os === "android" || Qt.platform.os === "ios")) {
                vpnFlickable.focus = true;
            }
            mouse.accepted = false;
        }
    }
}
