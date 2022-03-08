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
    property var windowHeightExceedsContentHeight: (window.safeContentHeight > flickContentHeight)
    property bool hideScollBarOnStackTransition: false

    clip: true

    function ensureVisible(item) {
        let yPosition = item.mapToItem(contentItem, 0, 0).y;
        if (windowHeightExceedsContentHeight || item.skipEnsureVisible || yPosition < 0) {
            return;
        }

        const buffer = 20;
        const itemHeight = Math.min(item.height, VPNTheme.theme.rowHeight) + buffer
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

    contentHeight: Math.max(window.safeContentHeight, flickContentHeight)
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
        anchors.rightMargin:  Qt.platform.os === "osx" ? 2 : 0

        background: Rectangle {
            color: "transparent"
            anchors.fill: parent
        }

        contentItem: Rectangle {
            color: VPNTheme.colors.grey40
            width: scrollBar.scrollBarWidth
            implicitWidth: scrollBar.scrollBarWidth
            radius: scrollBar.scrollBarWidth

            opacity: scrollBar.pressed ? 0.5 :
                   scrollBar.interactive && scrollBar.hovered ? 0.4 : 0.3
            Behavior  on opacity {
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
        visible: !windowHeightExceedsContentHeight

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

}
