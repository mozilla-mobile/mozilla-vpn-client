/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../themes/themes.js" as Theme

Flickable {
    property var flickContentHeight
    property var windowHeightExceedsContentHeight: (window.height > flickContentHeight)

    contentHeight: Math.max(window.height, flickContentHeight)
    boundsBehavior: Flickable.StopAtBounds
    opacity: 0
    Component.onCompleted: {
        opacity = 1;
    }

    Behavior on opacity {
        PropertyAnimation {
            duration: 200
        }

    }

    ScrollBar.vertical: ScrollBar {
        policy: windowHeightExceedsContentHeight ? ScrollBar.AlwaysOff : ScrollBar.AlwaysOn
        Accessible.ignored: true
    }

}
