/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNStackView {
    id: stackview

    function handleButtonClick() {
        VPNUrlOpener.openUrlLabel("subscriptionBlocked");
    }

    Component.onCompleted: {
        VPNNavigator.addStackView(VPNNavigator.ScreenSubscriptionBlocked, stackview)

        stackview.push("qrc:/ui/sharedViews/ViewErrorFullScreen.qml", {
            // "Problem confirming subscription…"
            headlineText: VPNl18n.MultiFxaAccountErrorFxaAccountErrorHeader,

            // "Your subscription is linked to another Firefox Account....."
            errorMessage: VPNl18n.MultiFxaAccountErrorFxaAccountErrorText,

            //% "Visit our help center to learn more about managing your subscriptions."
            errorMessage2: qsTrId("vpn.subscriptionBlocked.visitHelpCenter"),

            //% "Get Help"
            primaryButtonText: qsTrId("vpn.subscriptionBlocked.getHelp"),
            primaryButtonObjectName: "errorGetHelpButton",
            primaryButtonOnClick: stackview.handleButtonClick,
            secondaryButtonIsSignOff: true,
            getHelpLinkVisible: false
            }
        );
        MZGleanDeprecated.recordGleanEvent("subscriptionBlockedViewed");
        Glean.sample.subscriptionBlockedViewed.record();
    }
}
