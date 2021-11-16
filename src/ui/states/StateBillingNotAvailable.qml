/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

import org.mozilla.Glean 0.24
import telemetry 0.24

VPNStackView {
    id: stackview

    function handleButtonClick() {
        VPN.launchPlayStore()
    }

    Component.onCompleted: {
        stackview.push("qrc:/ui/views/ViewErrorFullScreen.qml", {
            // Sign in to Google Account
            headlineText: VPNl18n.NotSignedInGoogleGoogleModalHeader,

            // To continue subscribing, please sign in to your Google Account
            errorMessage: VPNl18n.NotSignedInGoogleGoogleModalBodyText,

            // Go to Play Store
            primaryButtonText: VPNl18n.NotSignedInGoogleGoogleModalLinkText,
            primaryButtonObjectName: "errorGetHelpButton",
            primaryButtonOnClick: stackview.handleButtonClick,
            secondaryButtonIsSignOff: true,
            getHelpLinkVisible: true
            }
        );
        Sample.billingNotAvailableViewed.record();
    }
}
