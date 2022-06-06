/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import components.inAppAuth 0.1

Item {
    property string _menuTitle: VPNl18n.SubscriptionManagementMenuTitle
    id: subscriptionManagementStates

    Loader {
        property bool _targetViewCondition: VPNProfileFlow.state === VPNProfileFlow.StateReady
        property string _targetViewSource: "qrc:/ui/settings/ViewSubscriptionManagement/ViewSubscriptionManagement.qml"

        source: "qrc:/ui/authenticationInApp/ViewReauthenticationFlow.qml"
        anchors.fill: parent
    }
}
