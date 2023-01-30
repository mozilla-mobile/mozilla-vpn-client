/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

import components 0.1


Item {
   id: root

   property string _menuTitle: ""
   property var _menuOnBackClicked
   property alias _viewContentData: viewContent.data
   property alias _interactive: vpnFlickable.interactive

   anchors {
       top: if (parent) parent.top
   }

   Rectangle {
       anchors.fill: root
       color: VPNTheme.theme.bgColor
   }

    VPNFlickable {
        id: vpnFlickable
        objectName: parent.objectName + "-flickable"

        anchors.fill: root
        flickContentHeight: viewContent.implicitHeight + viewContent.anchors.topMargin + viewContent.anchors.bottomMargin

        ColumnLayout {
            id: viewContent
            spacing: VPNTheme.theme.windowMargin
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                topMargin: VPNTheme.theme.windowMargin
                bottomMargin: navbar.visible ? 0 : VPNTheme.theme.rowHeight
            }
        }
    }

    function ensureVisible(itm) {
        vpnFlickable.ensureVisible(itm)
    }

    function setContentY(newY) {
        vpnFlickable.contentY = newY;
    }
}
