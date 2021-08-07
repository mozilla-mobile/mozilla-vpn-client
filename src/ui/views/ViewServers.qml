/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQml.Models 2.2
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme
import "../themes/colors.js" as Color

Item {
    id: root

    VPNMenu {
        id: menu
        objectName: "serverListBackButton"

        title: qsTrId("vpn.servers.selectLocation")
        onActiveFocusChanged: if (focus) forceFocus = true
    }

    ListModel {
        id: tabButtonList
        ListElement {
            buttonLabel: "String-Single"
        }
        ListElement {
            buttonLabel: "String-Multi"
        }
    }

    VPNTabNavigation {

        // TODO: add something real for multiHopEnabled
        property bool multiHopEnabled: true

        id: tabNavigation

        tabList: tabButtonList
        width: root.width
        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.height - menu.height

        stackContent: [
            VPNServerList {}
        ]
    }

    ViewMultiHop {
        id: multiHop
        visible: tabNavigation.multiHopEnabled
    }

    Component.onCompleted: {
      if (tabNavigation.multiHopEnabled) {
          tabNavigation.stackContent.push(multiHop);
      }
    }
}
