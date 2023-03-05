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
       VPNNavigator.addStackView(VPNNavigator.ScreenSubscriptionInUseError, stackView)

       stackView.push("qrc:/ui/sharedViews/ViewErrorFullScreen.qml", {
           // Problem confirming subscription...
           headlineText: MZI18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

           // Another Firefox Account has already subscribed using this Apple ID.
           // Visit our help center below to learn more about how to manage your subscriptions.
           errorMessage: MZI18n.RestorePurchaseInUseErrorRestorePurchaseInUseErrorText,

           // Sign out
           primaryButtonText: qsTrId("vpn.main.signOut2"),
           primaryButtonObjectName: "errorSignOutButton",
           primaryButtonOnClick: () => {
               VPNController.logout();
               stackView.pop();
           },
           secondaryButtonIsSignOff: false,
           getHelpLinkVisible: true
       });
    }
}
