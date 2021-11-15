/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import themes 0.1

Flickable {
    id: vpnFlickable

    property var flickContentHeight
    property var windowHeightExceedsContentHeight: (window.safeContentHeight > flickContentHeight)
    property bool hideScollBarOnStackTransition: false

    function ensureVisible(item) {
        let yPosition = item.mapToItem(contentItem, 0, 0).y;
        if (windowHeightExceedsContentHeight || item.skipEnsureVisible || yPosition < 0) {
            return;
        }

        const buffer = 20;
        const itemHeight = Math.min(item.height, Theme.rowHeight) + buffer
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
        policy: windowHeightExceedsContentHeight ? ScrollBar.AlwaysOff : ScrollBar.AlwaysOn
        Accessible.ignored: true
        opacity: hideScollBarOnStackTransition && (vpnFlickable.StackView.status !== StackView.Active) ? 0 : 1
        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

}
