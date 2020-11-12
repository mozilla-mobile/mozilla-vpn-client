/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

// VPNStabilityLabelAction
ParallelAnimation {
    property var connectionStatus: VPNConnectionHealth.stability
    property bool isConnectionUnstable: connectionStatus === VPNConnectionHealth.Unstable
    //% "Unstable"
    //: This refers to the user’s internet connection.
    readonly property var textUnable: qsTrId("vpn.connectionStability.unstable")
    //% "No Signal"
    readonly property var textNoSignal: qsTrId("vpn.connectionStability.noSignal")

    PropertyAction {
        target: stabilityLabel
        property: "text"
        value: isConnectionUnstable ? textUnable : textNoSignal
    }

    PropertyAction {
        target: warningIcon
        property: "source"
        value: isConnectionUnstable ? "../resources/warning-orange.svg" : "../resources/warning.svg"
    }

    PropertyAction {
        target: stabilityLabel
        property: "color"
        value: isConnectionUnstable ? Theme.orange : Theme.red
    }

}
