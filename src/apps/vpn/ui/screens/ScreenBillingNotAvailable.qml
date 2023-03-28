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
        MZUtils.launchPlayStore()
    }

    Component.onCompleted: {
        MZNavigator.addStackView(VPN.ScreenBillingNotAvailable, stackview)

        stackview.push("qrc:/ui/sharedViews/ViewErrorFullScreen.qml", {
            // Sign in to Google Account
            headlineText: MZI18n.NotSignedInGoogleGoogleModalHeader,

            // To continue subscribing, please sign in to your Google Account
            errorMessage: MZI18n.NotSignedInGoogleGoogleModalBodyText,

            // Go to Play Store
            primaryButtonText: MZI18n.NotSignedInGoogleGoogleModalLinkText,
            primaryButtonObjectName: "errorGetHelpButton",
            primaryButtonOnClick: stackview.handleButtonClick,
            secondaryButtonIsSignOff: true,
            getHelpLinkVisible: true
            }
        );
        MZGleanDeprecated.recordGleanEvent("billingNotAvailableViewed");
        Glean.sample.billingNotAvailableViewed.record();
    }
}
