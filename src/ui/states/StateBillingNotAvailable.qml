/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15

VPNStackView {
    id: stackview

    function handleButtonClick() {
        // TODO - Implement launchPlayStore()
        VPN.launchPlayStore()
    }

    Component.onCompleted: {
        stackview.push("../views/ViewErrorFullScreen.qml", {
            // Sign in to Google Account
            headlineText: VPNl18n.tr(VPNl18n.NotSignedInGoogleGoogleModalHeader),

            // To continue subscribing, please sign in to your Google Account
            errorMessage: VPNl18n.tr(VPNl18n.NotSignedInGoogleGoogleModalBodyText),

            // Go to Play Store
            buttonText: VPNl18n.tr(VPNl18n.NotSignedInGoogleGoogleModalLinkText),
            buttonObjectName: "errorGetHelpButton",
            buttonOnClick: stackview.handleButtonClick,
            signOffLinkVisible: true,
            getHelpLinkVisible: true
            }
        );
    }
}
