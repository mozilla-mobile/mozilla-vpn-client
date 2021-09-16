/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.20
import telemetry 0.20

VPNStackView {
    id: stackview

    function handleButtonClick() {
        VPN.reset(true);
    }

    Component.onCompleted: {
        stackview.push("../views/ViewErrorFullScreen.qml", {
            // "Problem confirming subscription…"
            headlineText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

            // "An unexpected error....."
            errorMessage: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorText,

            // "Try again"
            buttonText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorButton,
            buttonObjectName: "errorGetHelpButton",
            buttonOnClick: stackview.handleButtonClick,
            signOffLinkVisible: false,
            getHelpLinkVisible: true,
            }
        );
        Sample.subNotValidatedViewed.record();
    }
}
