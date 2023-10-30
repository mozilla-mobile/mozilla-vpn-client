/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZLoader {
    objectName: "subscriptionInProgressWeb"
    headlineText: MZI18n.PurchaseWebInProgress2

    // Hack: The Component.OnCompleted event was not working for me (Bea).
    // So I am running this on property initialization.
    property var _: {
        Glean.impression.continueInBrowserScreen.record({
            screen: telemetryScreenId,
        });
    }

    property string telemetryScreenId: "continue_in_browser"

    MZCancelButton {
        id: footerLink
        objectName: "cancelFooterLink"

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Math.min(window.safeContentHeight * .08, 60)

        visible: footerLinkIsVisible
        onClicked: {
            Glean.interaction.cancelSelected.record({
                screen: telemetryScreenId,
            });

            VPNPurchase.cancelSubscription();
        }
    }
}
