/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15

VPNStackView {
    id: stackview

    function handleButtonClick() {
        VPN.openLink(VPN.LinkSubscriptionsBlocked)
    }

    Component.onCompleted: {
        stackview.push("../views/ViewErrorFullScreen.qml", {
            //% "Error confirming subscription…"
            headlineText: qsTrId("vpn.subscriptionBlocked.title"),

            //% "Another Firefox Account has already subscribed using this Apple ID."
            errorMessage:qsTrId("vpn.subscriptionBlocked.anotherFxaSubscribed"),

            //% "Visit our help center to learn more about managing your subscriptions."
            errorMessage2: qsTrId("vpn.subscriptionBlocked.visitHelpCenter"),

            //% "Get Help"
            buttonText: qsTrId("vpn.subscriptionBlocked.getHelp"),
            buttonObjectName: "errorGetHelpButton",
            buttonOnClick: stackview.handleButtonClick,
            signOffLinkVisible: true,
            getHelpLinkVisible: false
            }
        );
        Sample.subscriptionBlockedViewed.record();
    }
}
