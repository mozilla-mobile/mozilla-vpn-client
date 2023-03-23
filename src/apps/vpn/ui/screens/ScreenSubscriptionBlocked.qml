/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZStackView {
    id: stackview

    function handleButtonClick() {
        MZUrlOpener.openUrlLabel("subscriptionBlocked");
    }

    Component.onCompleted: {
        MZNavigator.addStackView(VPN.ScreenSubscriptionBlocked, stackview)

        stackview.push("qrc:/ui/sharedViews/ViewErrorFullScreen.qml", {
            // "Problem confirming subscription…"
            headlineText: MZI18n.MultiFxaAccountErrorFxaAccountErrorHeader,

            // "Your subscription is linked to another Firefox Account....."
            errorMessage: MZI18n.MultiFxaAccountErrorFxaAccountErrorText,

            //% "Visit our help center to learn more about managing your subscriptions."
            errorMessage2: qsTrId("vpn.subscriptionBlocked.visitHelpCenter"),

            primaryButtonText: MZI18n.GetHelpLinkTitle,
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
