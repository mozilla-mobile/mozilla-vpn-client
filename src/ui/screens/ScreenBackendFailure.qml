/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZStackView {
    id: stackview

    function handleButtonClick() {
        VPN.triggerHeartbeat();
    }

    Component.onCompleted: {
        MZNavigator.addStackView(VPN.ScreenBackendFailure, stackview)

        stackview.push(
            "qrc:/Mozilla/VPN/sharedViews/ViewErrorFullScreen.qml", {
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

        Glean.sample.backendFailureViewed.record();
    }
}
