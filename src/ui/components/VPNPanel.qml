/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Item {
    id: panel

    property alias logo: logo.source
    property alias logoTitle: logoTitle.text
    property alias logoSubtitle: logoSubtitle.text
    property var logoSize: 76
    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width

    Rectangle {
        id: contentWrapper

        height: contentWrapper.childrenRect.height
        anchors.centerIn: panel

        Rectangle {
            // We nest the panel Image inside this Rectangle to prevent
            // logoTitle and logoSubittle from wiggling when Image.height
            // changes as Image.src is updated.

            id: logoWrapper

            color: "transparent"
            height: logoSize
            width: logoSize
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                id: logo

                anchors.horizontalCenter: parent.horizontalCenter
                sourceSize.height: logoSize
                fillMode: Image.PreserveAspectFit
            }

        }

        VPNHeadline {
            id: logoTitle

            anchors.top: logoWrapper.bottom
            anchors.topMargin: 24
        }

        VPNSubtitle {
            id: logoSubtitle

            anchors.top: logoTitle.bottom
            anchors.topMargin: 12
        }

    }

}
