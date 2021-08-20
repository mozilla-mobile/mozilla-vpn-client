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

        title: defaultMenuTitle
        onActiveFocusChanged: if (focus) forceFocus = true
        isMultiHopView: true

        function handleMultiHopNav() {
            if (!returnToMultiHopMainView()) {
                return stackview.pop();
            }
        }
    }

    ListModel {
        id: tabButtonList
        ListElement {
            tabLabelStringId: "MultiHopFeatureSingleToggleHeader"
            tabButtonId: "tabButtonSingleHop"
        }
        ListElement {
            tabLabelStringId: "MultiHopFeatureMultiToggleHeader"
            tabButtonId: "tabButtonMultiHeader"
        }
    }

    VPNTabNavigation {
        // MULTIHOP TODO - Open server view to correct tab based on number of hops

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
        visible: VPNFeatureList.multihopSupported
    }


    Component.onCompleted: {
      if (VPNFeatureList.multihopSupported) {
          tabNavigation.stackContent.push(multiHopStackView);
      }

      if (VPNSettings.multihopTunnel) {
          // Set default tab to multi-hop
          tabNavigation.setCurrentTabIndex(1)
      }
    }
}

