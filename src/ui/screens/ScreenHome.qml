/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import components 0.1

VPNStackView {
    id: stackview
    objectName: "ViewMainStackView"

    Component.onCompleted: function(){
        stackview.push("qrc:/ui/views/ViewMain.qml")
    }

    Connections {
        target: window
        function onShowServerList() {
            // We get here after the user clicks the "Choose new location" button in VPNServerUnavailablePopup {}
            // We need to (maybe) unwind the stack back to ViewMain.qml and then push the server list.
            if (stackview.currentItem.objectName === "viewServers") {
                // User is already on server list view so we stay put
                return;
            }
            stackview.unwindToInitialItem();
            stackview.push("qrc:/ui/views/ViewServers.qml", StackView.Immediate)
        }
    }
}
