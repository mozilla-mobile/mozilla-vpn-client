/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../themes/themes.js" as Theme

Flickable {
    id: vpnFlickable

    property var flickContentHeight
    property var windowHeightExceedsContentHeight: (window.height > flickContentHeight)

    function ensureVisible(item) {
        if (windowHeightExceedsContentHeight) {
            return;
        }

        let yPosition = item.mapToItem(contentItem, 0, 0).y;
        let ext = item.height + yPosition;
        if (yPosition < contentY || yPosition > contentY + height || ext < contentY || ext > contentY + height) {
            let destinationY = Math.max(0, Math.min(yPosition - height + item.height, contentHeight - height));
            ensureVisAnimation.to = destinationY;
            ensureVisAnimation.start();
        }
    }

    contentHeight: Math.max(window.height, flickContentHeight)
    boundsBehavior: Flickable.StopAtBounds
    opacity: 0
    Component.onCompleted: {
        opacity = 1;
    }

    NumberAnimation on contentY {
        id: ensureVisAnimation

        to: 0 //Dummy value - will be set up when this animation is called.
        duration: 300
        easing.type: Easing.OutQuad
    }

    PropertyAnimation on opacity {
        duration: 200
    }

    ScrollBar.vertical: ScrollBar {
        policy: windowHeightExceedsContentHeight ? ScrollBar.AlwaysOff : ScrollBar.AlwaysOn
        Accessible.ignored: true
    }

}
