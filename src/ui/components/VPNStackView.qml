/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0


StackView {
    id: stackView
    Component.onCompleted: VPNCloseEventHandler.addStackView(stackView)

    onCurrentItemChanged: {
        var objString = currentItem.toString().split("(")[0];
        VPN.currentView = objString.split("_QML")[0];
    }

    anchors.fill: parent

    Connections {
        target: VPNCloseEventHandler
        function onGoBack(item) {
            if (item === stackView) {
                stackView.pop();
            }
        }
    }
}
