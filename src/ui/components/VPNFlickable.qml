/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import "../themes/themes.js" as Theme

Flickable {
    id: vpnFlickable

    property var flickContentHeight
    property var windowHeightExceedsContentHeight: (window.safeContentHeight > flickContentHeight)
    property bool hideScollBarOnStackTransition: false

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

    contentHeight: Math.max(window.safeContentHeight, flickContentHeight)
    boundsBehavior: Flickable.StopAtBounds
    opacity: 0
    Component.onCompleted: {
        opacity = 1;
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
