/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.4

import Mozilla.VPN 1.0


StackView {
    id: stackView
    Component.onCompleted: VPNCloseEventHandler.addStackView(stackView)

    Connections {
        target: VPNCloseEventHandler
        function onGoBack(item) {
            if (item === stackView) {
                stackView.pop();
            }
        }
    }
}
