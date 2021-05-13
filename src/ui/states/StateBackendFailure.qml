/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import Mozilla.VPN 1.0
import "../components"


VPNStackView {
    id: stackview

    function handleButtonClick() {
        VPN.triggerHeartbeat();
    }

    Component.onCompleted:stackview.push(
        "../views/ViewErrorFullScreen.qml",
        {
            //% "Something went wrong…"
            headlineText: qsTrId("vpn.errors.somethingWentWrong"),

            //% "Unable to establish a connection at this time. We’re working hard to resolve the issue. Please try again shortly."
            errorMessage: qsTrId("vpn.errors.unableToEstablishConnection"),

            //% "Check for outage updates on our <a href='https://vpn.status.mozilla.org'>status page</a>."
            errorMessage2: qsTrId("vpn.errors.seeStatusPage"),

            //% "Try Again"
            buttonText: qsTrId("vpn.errors.tryAgain"),

            buttonObjectName: "heartbeatTryButton",
            buttonOnClick: stackview.handleButtonClick,
            signOffLinkVisible: false,
            getHelpLinkVisible: true
        }
    )
}
