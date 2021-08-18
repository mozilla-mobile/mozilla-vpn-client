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

    function returnToMultiHopMainView() {
        if (multiHopStackView && multiHopStackView.depth > 1) {
            multiHopStackView.pop();
            menu.title = menu.defaultMenuTitle;
            return true;
        }
        return false;
    }

    VPNMenu {
        property string defaultMenuTitle: qsTrId("vpn.servers.selectLocation")
        id: menu
        objectName: "serverListBackButton"

        title: qsTrId("vpn.servers.selectLocation")
        onActiveFocusChanged: if (focus) forceFocus = true
        isMultiHopView: true

        function handleMultiHopNav() {
            if (!returnToMultiHopMainView()) {
                return stackview.pop();
            }
        }
    }

    ListModel {
        // MULTIHOP TODO - Use real strings
        id: tabButtonList
        ListElement {
            buttonLabel: "String-Single"
            objectId: "tabSingleHop"
        }
        ListElement {
            buttonLabel: "String-Multi"
            objectId: "tab"
        }
    }

    VPNTabNavigation {
        // MULTIHOP TODO - Actually detect if multiHop is enabled
        // MULTIHOP TODO - Open server view to correct tab based on number of hops
        property bool multiHopEnabled: true

        // Reset menu title when the tab view is changed
        onStackChange: returnToMultiHopMainView();

        id: tabNavigation
        tabList: tabButtonList
        width: root.width
        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.height - menu.height

        stackContent: [
            VPNServerList {
                showRecentConnections: true
            }
        ]
    }

    ViewMultiHop {
        id: multiHopStackView
        visible: tabNavigation.multiHopEnabled
    }

    Component.onCompleted: {
      if (tabNavigation.multiHopEnabled) {
          tabNavigation.stackContent.push(multiHopStackView);
      }
    }
}
