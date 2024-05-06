/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZStackView {
    id: stackView

    Component.onCompleted: function(){
       MZNavigator.addStackView(VPN.ScreenSubscriptionExpiredError, stackView)

       stackView.push("qrc:/Mozilla/VPN/sharedViews/ViewErrorFullScreen.qml", {
           // Problem confirming subscription...
           headlineText: MZI18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

           // Sorry we are unable to connect your Mozilla account to a current subscription.
           // Please try again or contact our support team for further assistance.
           errorMessage: MZI18n.RestorePurchaseExpiredErrorRestorePurchaseExpiredErrorText2,

           // Try again (Error SubscriptionExpiredError only happens on iOS, so ok to point to ScreenSubscriptionNeeded)
           primaryButtonText: MZI18n.GenericPurchaseErrorGenericPurchaseErrorButton,
           primaryButtonObjectName: "errorTryAgainButton",
           primaryButtonOnClick: () => { MZNavigator.requestScreen(VPN.ScreenSubscriptionNeeded) },
           secondaryButtonIsSignOff: false,
           getHelpLinkVisible: true
       });
    }
}

