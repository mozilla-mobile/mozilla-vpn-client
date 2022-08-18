/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import components 0.1


Item {
   id: root
   property string _menuTitle: ""
   property var _menuOnBackClicked
   property alias _viewContentData: viewContent.data

   Rectangle {
       anchors.fill: root
       color: VPNTheme.theme.bgColor
   }

   function ensureVisible(itm) {
       vpnFlickable.ensureVisible(itm)
   }

   function setContentY(newY) {
       vpnFlickable.contentY = newY;
   }

    VPNFlickable {
        id: vpnFlickable

        flickContentHeight: viewContent.implicitHeight + anchors.topMargin
        interactive: viewContent.implicitHeight > height - anchors.topMargin

        anchors {
            fill: root
            topMargin: _menuTitle === "" ? 0 : VPNTheme.theme.menuHeight
        }

        ColumnLayout {
            id: viewContent
            spacing: VPNTheme.theme.windowMargin
            anchors {
                top: parent.top
                topMargin: VPNTheme.theme.windowMargin
                left: parent.left
                right: parent.right
            }
        }
    }
}
