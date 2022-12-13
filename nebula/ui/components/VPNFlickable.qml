/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

Flickable {
    id: vpnFlickable

    property var flickContentHeight
    property bool contentExceedsHeight: height < contentHeight
    property bool hideScollBarOnStackTransition: false
    //This property should be true if the flickable appears behind the main navbar
    interactive: !VPNTutorial.playing && contentHeight > height || contentY > 0

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

    clip: true

    function ensureVisible(item) {
        let yPosition = item.mapToItem(contentItem, 0, 0).y;
        if (!contentExceedsHeight || item.skipEnsureVisible || yPosition < 0) {
            return;
        }

        const buffer = 20;
        const itemHeight = Math.max(item.height, VPNTheme.theme.rowHeight) + buffer
        let ext = item.height + yPosition;
        let destinationY;


        if (yPosition < vpnFlickable.contentY || yPosition > vpnFlickable.contentY + vpnFlickable.height || ext < vpnFlickable.contentY || ext > vpnFlickable.contentY + vpnFlickable.height) {
            destinationY = Math.min(yPosition - vpnFlickable.height + itemHeight, vpnFlickable.contentHeight - vpnFlickable.height);
        }

        if (yPosition < vpnFlickable.contentY) {
            const diff = vpnFlickable.contentY - yPosition;
            if (diff < itemHeight) {
                destinationY = vpnFlickable.contentY - (vpnFlickable.contentY - yPosition);
                destinationY = (destinationY - buffer) > 0 ? (destinationY - buffer) : destinationY;
            }

        }
        if (typeof(destinationY) === "undefined") return;

        ensureVisAnimation.to = destinationY > 0 ? destinationY : 0;
        ensureVisAnimation.start();
    }

    function recalculateContentHeight() {
        //Absolute y coordinate position of the scroll view
        const absoluteYPosition = mapToItem(window.contentItem, 0, 0).y
        //Portion of the screen that a view's contents can reside without interfering with the navbar
        const contentSpace = window.height - VPNTheme.theme.navBarHeightWithMargins

        //Checks if the flickable AND it's content interferes with the navbar area (bottom 146px for iOS, bottom 128px for other platforms)
        //If it does, we pad the flickable's contentHeight with whatever bottom padding is needed so that there is always 48px (aka theme.navBarTopMargin)
        //between the bottom of the flickable's content and the navbar
        if (navbar.visible && absoluteYPosition + height >= contentSpace
                && flickContentHeight + absoluteYPosition >= contentSpace) {
            vpnFlickable.contentHeight = flickContentHeight + (flickContentHeight >= height ? VPNTheme.theme.navBarHeightWithMargins : (absoluteYPosition + flickContentHeight) - contentSpace + (height - flickContentHeight))
        }
        //If the navbar isn't visible, or the flickable's content does not interfere with the navbar area, don't worry about adding any padding
        else {
            vpnFlickable.contentHeight = flickContentHeight
        }
    }

    onFlickContentHeightChanged: {
        recalculateContentHeight()
    }

    onHeightChanged: {
        recalculateContentHeight()
    }

    boundsBehavior: Flickable.StopAtBounds
    opacity: 0

    Component.onCompleted: {
        opacity = 1;
        if (Qt.platform.os === "windows") {
            maximumFlickVelocity = 700;
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
            color: VPNTheme.theme.transparent
            anchors.fill: parent
        }

        contentItem: Rectangle {
            color: VPNTheme.colors.grey40
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

}
