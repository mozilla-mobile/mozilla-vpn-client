/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZScreenBase {
    id: stackview
    objectName: "screenPermissionRequired"

    _menuIconVisibility: false

    Component.onCompleted: () => {
        MZNavigator.addStackView(VPN.ScreenPermissionRequired, getStack())
        if(Qt.platform.os === "osx"){
            getStack().push("qrc:/qt/qml/Mozilla/VPN/sharedViews/ViewPermissionRequiredOSX.qml")
            return;
        }
        // TODO: What to display otherwise?
    }
}
