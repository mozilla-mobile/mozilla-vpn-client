///* This Source Code Form is subject to the terms of the Mozilla Public
// * License, v. 2.0. If a copy of the MPL was not distributed with this
// * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1


MZStackView {
    id: stackview
    Component.onCompleted: () => {
       MZNavigator.addStackView(VPN.ScreenDeleteAccount, stackview)
       stackview.push("qrc:/qt/qml/Mozilla/VPN/deleteAccount/ViewDeleteAccount.qml")
    }
}
