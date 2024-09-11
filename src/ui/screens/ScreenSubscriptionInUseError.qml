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
       MZNavigator.addStackView(VPN.ScreenSubscriptionInUseError, stackView)

       stackView.push("qrc:/qt/qml/Mozilla/VPN/sharedViews/ViewErrorFullScreen.qml", {
           // Problem confirming subscription...
           headlineText: MZI18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

           // Another Mozilla account has already subscribed using this Apple ID.
           // Visit our help center below to learn more about how to manage your subscriptions.
           errorMessage: MZI18n.RestorePurchaseInUseErrorRestorePurchaseInUseErrorText2,

           // Sign out
           primaryButtonText: MZI18n.GlobalSignOut,
           primaryButtonObjectName: "errorSignOutButton",
           primaryButtonOnClick: () => {
               VPN.logout();
               stackView.pop();
           },
           secondaryButtonIsSignOff: false,
           getHelpLinkVisible: true
       });
    }
}
