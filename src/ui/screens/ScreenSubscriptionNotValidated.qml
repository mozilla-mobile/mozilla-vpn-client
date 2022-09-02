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
        VPN.reset(true);
    }

    Component.onCompleted: {
        VPNNavigator.addStackView(VPNNavigator.ScreenSubscriptionNotValidated, stackview)

        stackview.push("qrc:/ui/sharedViews/ViewErrorFullScreen.qml", {
            // "Problem confirming subscription…"
            headlineText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

            // "An unexpected error....."
            errorMessage: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorText,

            // "Try again"
            primaryButtonText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorButton,
            primaryButtonObjectName: "errorGetHelpButton",
            primaryButtonOnClick: stackview.handleButtonClick,
            secondaryButtonIsSignOff: false,
            getHelpLinkVisible: true,
            }
        );
        VPN.recordGleanEvent("subNotValidatedViewed");
    }
}
