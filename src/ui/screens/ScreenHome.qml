/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZScreenBase {
    objectName: "screenHome"
    id: screenHome
    Accessible.name: MZI18n.ProductName
    Accessible.role: Accessible.Client

    Component.onCompleted: () => {
        MZNavigator.addStackView(VPN.ScreenHome, getStack())
        getStack().push("qrc:/qt/qml/Mozilla/VPN/screens/home/ViewHome.qml")
    }

    Connections {
        target: window

        // TODO - This only works when we are already in ScreenHome
        function onShowServerList() {

            // Don't push another server view if it's already open
            if (getStack().currentItem.objectName === "viewServers") { return; }

            // unwind stackview to initial view
            getStack().pop(null);

            // push server view
            getStack().push("qrc:/qt/qml/Mozilla/VPN/screens/home/ViewServers.qml", StackView.Immediate);
        }
    }

    onVisibleChanged: {
        // Screens with LoadPersistently policy have their visiblity turned on when they become the
        // current screen. Reset focus to the root to clear the previous focus.
        if (visible) {
            screenHome.forceActiveFocus();
        }
    }
}
