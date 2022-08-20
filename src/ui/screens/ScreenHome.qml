/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    objectName: "ViewMainStackView"

    VPNMenu {
        id: menu
        objectName: "ViewMainBack"
        _menuOnBackClicked: () => {
            if (stackview.depth !== 1) {
                return stackview.pop();
            }
            VPNNavigator.requestPreviousScreen();
        }
        _iconButtonSource: stackview.depth === 1 ? "qrc:/nebula/resources/close-dark.svg" : "qrc:/nebula/resources/back.svg"
        _iconButtonAccessibleName: stackview.depth === 1 ? qsTrId("vpn.connectionInfo.close") : qsTrId("vpn.main.back")
    }

    VPNStackView {
        id: stackview
        anchors.top: menu.bottom

        Component.onCompleted: {
            VPNNavigator.addStackView(VPNNavigator.ScreenHome, stackview)
            stackview.push("qrc:/ui/screens/home/ViewHome.qml")
        }

        onCurrentItemChanged: {
            menu.title = Qt.binding(() => currentItem._menuTitle || "");
            menu.visible = Qt.binding(() => menu.title !== "");
            menu._menuOnBackClicked = currentItem._menuOnBackClicked ? currentItem._menuOnBackClicked : () => stackview.pop()
        }
    }

    Connections {
        target: window
        function onShowServerList() {
            // We get here after the user clicks the "Choose new location" button in VPNServerUnavailablePopup {}
            // We need to (maybe) unwind the stack back to ViewHome.qml and then push the server list.
            if (stackview.currentItem.objectName === "viewServers") {
                // User is already on server list view so we stay put
                return;
            }
            stackview.unwindToInitialItem();
            stackview.push("qrc:/ui/screens/home/ViewServers.qml", StackView.Immediate)
        }
    }

    Connections {
        target: VPNTutorial
        function onPlayingChanged() {
           if (VPNTutorial.playing) {
               stackview.pop(null, StackView.Immediate)
           }
        }
    }
}
