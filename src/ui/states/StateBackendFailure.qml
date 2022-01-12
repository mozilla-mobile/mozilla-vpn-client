/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


VPNStackView {
    id: stackview

    function handleButtonClick() {
        VPN.triggerHeartbeat();
    }
    Component.onCompleted: {
        stackview.push(
            "qrc:/ui/views/ViewErrorFullScreen.qml", {
                //% "Something went wrong…"
                headlineText: qsTrId("vpn.errors.somethingWentWrong"),

                //% "Unable to establish a connection at this time. We’re working hard to resolve the issue. Please try again shortly."
                errorMessage: qsTrId("vpn.errors.unableToEstablishConnection"),

                //% "Try Again"
                primaryButtonText: qsTrId("vpn.errors.tryAgain"),

                primaryButtonObjectName: "heartbeatTryButton",
                primaryButtonOnClick: stackview.handleButtonClick,
                secondaryButtonIsSignOff: false,
                getHelpLinkVisible: true,
                statusLinkVisible: true
            }
        );
        Sample.backendFailureViewed.record();
    }
}
