/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1


Item {
   id: root

   property string _menuTitle: ""
   property string _accessibleName: ""
   property var _menuOnBackClicked
   property alias _viewContentData: viewContent.data
   property alias _interactive: vpnFlickable.interactive
   property alias _contentHeight: vpnFlickable.contentHeight

   Accessible.name: (_menuTitle.length > 0) ? _menuTitle : _accessibleName
   Accessible.role: Accessible.Pane

   anchors {
       top: if (parent) parent.top
   }

   Rectangle {
       anchors.fill: root
       color: MZTheme.theme.bgColor
   }

    MZFlickable {
        id: vpnFlickable
        objectName: parent.objectName + "-flickable"

        anchors.fill: root
        flickContentHeight: viewContent.childrenRect.height + viewContent.anchors.topMargin + viewContent.anchors.bottomMargin

        ColumnLayout {
            id: viewContent
            spacing: MZTheme.theme.windowMargin
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                topMargin: MZTheme.theme.viewBaseTopMargin
                bottomMargin: navbar.visible ? 0 : MZTheme.theme.rowHeight
            }
        }
    }

    onVisibleChanged: {
        // Screens with LoadPersistently policy have their visiblity turned on when they become the
        // current screen. Reset focus to the root to clear the previous focus.
        if (visible) {
            root.forceActiveFocus();
        }
    }

    function ensureVisible(itm) {
        vpnFlickable.ensureVisible(itm)
    }

    function setContentY(newY) {
        vpnFlickable.contentY = newY;
    }
}
