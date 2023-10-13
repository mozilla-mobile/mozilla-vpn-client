/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import components 0.1
import QtQuick 2.15

MZLoader {
    objectName: "subscriptionInProgressIAP"

    headlineText: MZI18n.InAppAuthPleaseWait
    footerLinkIsVisible: false

    Component.onCompleted: {
        Glean.impression.confirmingSubscriptionScreen.record({
            screen: "confirming_subscription",
        });
    }
}
