/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

VPNScreenBase {
    objectName: "screenHome"
    _onStackCompleted: () => {
        VPNNavigator.addStackView(VPNNavigator.ScreenHome, getStack())
        getStack().push("qrc:/ui/screens/home/ViewHome.qml")
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
               getStack().pop(null, StackView.Immediate)
           }
        }
    }
}
