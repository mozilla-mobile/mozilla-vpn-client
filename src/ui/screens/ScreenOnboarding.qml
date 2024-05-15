/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZScreenBase {
    objectName: "screenOnboarding"

    Component.onCompleted: () => {
        MZNavigator.addStackView(VPN.ScreenOnboarding, getStack())
        getStack().push("qrc:/qt/qml/Mozilla/VPN/screens/onboarding/ViewOnboarding.qml")
    }
}
